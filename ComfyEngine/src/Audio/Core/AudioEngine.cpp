#include "AudioEngine.h"
#include "AudioInstance.h"
#include "Audio/Decoder/AudioDecoderFactory.h"
#include "Core/Logger.h"
#include <RtAudio.h>
#include <functional>
#include <assert.h>

namespace Comfy::Audio
{
	namespace
	{
		constexpr TimeSpan FramesToTimeSpan(i64 frames, i64 sampleRate)
		{
			return TimeSpan::FromSeconds(static_cast<f64>(frames) / static_cast<f64>(sampleRate));
		}

		constexpr i64 TimeSpanToFrames(TimeSpan time, i64 sampleRate)
		{
			return static_cast<i64>(time.TotalSeconds() * static_cast<f64>(sampleRate));
		}
	}

	std::unique_ptr<AudioEngine> AudioEngineInstance = nullptr;

	enum class AudioCallbackResult
	{
		Continue = 0,
		Stop = 1,
	};

	using VoiceFlags = u16;
	enum VoiceFlagsEnum : VoiceFlags
	{
		VoiceFlags_Dead = 0,
		VoiceFlags_Alive = 1 << 0,
		VoiceFlags_Playing = 1 << 1,
		VoiceFlags_Looping = 1 << 2,
		VoiceFlags_PlayPastEnd = 1 << 3,
		VoiceFlags_RemoveOnEnd = 1 << 4,
	};

	// NOTE: POD reusable voice instance internal data
	struct VoiceData
	{
		// NOTE: Automatically resets to SourceHandle::Invalid when the source is unloaded
		VoiceFlags Flags;
		SourceHandle Source;
		f32 Volume;
		i64 FramePosition;
		std::array<char, 64> Name;

		// TODO: TimeSpan FadeInTime, FadeOutTime;
	};

	static_assert(std::is_pod_v<VoiceData>);

	struct AudioEngine::Impl
	{
	public:
		bool IsStreamOpen = false, IsStreamRunning = false;
		f32 MasterVolume = AudioEngine::MaxVolume;

	public:
		std::mutex CallbackMutex;

		// NOTE: Indexed into by VoiceHandle, nullptr = free space
		std::array<VoiceData, MaxSimultaneousVoices> ActiveVoices;

		// NOTE: Indexed into by SourceHandle, nullptr = free space
		std::vector<std::unique_ptr<ISampleProvider>> LoadedSources;

	public:
		static constexpr size_t TempOutputBufferSize = (MaxSampleBufferSize * OutputChannelCount);

		std::array<i16, TempOutputBufferSize> TempOutputBuffer = {};
		u32 CurrentBufferFrameSize = DefaultSampleBufferSize;

		TimeSpan CallbackLatency;
		TimeSpan CallbackStreamTime, LastCallbackStreamTime;

		// NOTE: nullptr = free space
		std::vector<ICallbackReceiver*> RegisteredCallbackReceivers;

	public:
		AudioAPI CurrentAudioAPI = AudioAPI::Invalid;
		ChannelMixer ActiveChannelMixer;

	public:
		struct RtAudioData
		{
			std::unique_ptr<RtAudio> Context = nullptr;
			RtAudio::StreamParameters StreamOutputParameter;

			RtAudio::Api GetRtAudioAPI(AudioAPI inputAPI)
			{
				switch (inputAPI)
				{
				case AudioAPI::ASIO:
					return RtAudio::WINDOWS_ASIO;
				case AudioAPI::WASAPI:
					return RtAudio::WINDOWS_WASAPI;
				default:
					assert(false);
				case AudioAPI::Invalid:
					return RtAudio::UNSPECIFIED;
				}
			}

		} RtAudio;

	public:
		VoiceData* GetVoiceData(VoiceHandle handle)
		{
			auto voice = IndexOrNull(static_cast<HandleBaseType>(handle), ActiveVoices);
			return (voice != nullptr && (voice->Flags & VoiceFlags_Alive)) ? voice : nullptr;
		}

		ISampleProvider* GetSource(SourceHandle handle)
		{
			auto sourcePtr = IndexOrNull(static_cast<HandleBaseType>(handle), LoadedSources);
			return (sourcePtr != nullptr && *sourcePtr != nullptr) ? sourcePtr->get() : nullptr;
		}

		void AudioCallbackNotifyCallbackReceivers()
		{
			for (auto* callbackReceiver : RegisteredCallbackReceivers)
			{
				if (callbackReceiver != nullptr)
					callbackReceiver->OnAudioCallback();
			}
		}

		void AudioCallbackClearOutPreviousCallBuffer(i16* outputBuffer, size_t sampleCount)
		{
			std::fill(outputBuffer, outputBuffer + sampleCount, 0);
		}

		void AudioCallbackProcessVoices(i16* outputBuffer, const u32 bufferFrameCount, const u32 bufferSampleCount)
		{
			const auto lock = std::scoped_lock(CallbackMutex);

			// NOTE: 2 channels * 64 frames * sizeof(i16) -> 256 bytes inside the outputBuffer
			//		 64 samples for each ear

			for (auto& voiceData : ActiveVoices)
			{
				if (!(voiceData.Flags & VoiceFlags_Alive))
					continue;

				auto sampleProvider = GetSource(voiceData.Source);
				if (sampleProvider == nullptr)
					continue;

				const bool isPlaying = (voiceData.Flags & VoiceFlags_Playing);
				const bool isLooping = (voiceData.Flags & VoiceFlags_Looping);
				const bool playPastEnd = (voiceData.Flags & VoiceFlags_PlayPastEnd);
				const bool removeOnEnd = (voiceData.Flags & VoiceFlags_RemoveOnEnd);

				const bool hasReachedEnd = (voiceData.FramePosition >= sampleProvider->GetFrameCount());

				if (!playPastEnd && removeOnEnd && hasReachedEnd)
				{
					voiceData.Flags = VoiceFlags_Dead;
					continue;
				}

				if (!isPlaying || voiceData.Volume <= 0.0f)
					continue;

				if (hasReachedEnd && !playPastEnd)
					voiceData.FramePosition = isLooping ? 0 : sampleProvider->GetFrameCount();

				i64 framesRead = 0;
				const auto sampleproviderChannels = sampleProvider->GetChannelCount();

				if (sampleproviderChannels != OutputChannelCount)
				{
					ActiveChannelMixer.SetSourceChannels(sampleproviderChannels);
					framesRead = ActiveChannelMixer.MixChannels(sampleProvider, TempOutputBuffer.data(), voiceData.FramePosition, bufferFrameCount);
				}
				else
				{
					framesRead = sampleProvider->ReadSamples(TempOutputBuffer.data(), voiceData.FramePosition, bufferFrameCount, OutputChannelCount);
				}

				voiceData.FramePosition += framesRead;

				for (i64 i = 0; i < (framesRead * OutputChannelCount); i++)
					outputBuffer[i] = SampleMixer::MixSamples(outputBuffer[i], static_cast<i16>(TempOutputBuffer[i] * voiceData.Volume));
			}
		}

		void AudioCallbackAdjustBufferMasterVolume(i16* outputBuffer, size_t sampleCount)
		{
			const auto bufferSampleCount = (CurrentBufferFrameSize * OutputChannelCount);
			for (auto i = 0; i < sampleCount; i++)
				outputBuffer[i] = static_cast<i16>(outputBuffer[i] * MasterVolume);
		}

		AudioCallbackResult AudioCallback(i16* outputBuffer, u32 bufferFrameCount, double streamTime)
		{
			const auto bufferSampleCount = (bufferFrameCount * OutputChannelCount);
			CurrentBufferFrameSize = bufferFrameCount;

			CallbackStreamTime = TimeSpan::FromSeconds(streamTime);
			LastCallbackStreamTime = CallbackStreamTime;

			CallbackLatency = (CallbackStreamTime - LastCallbackStreamTime);

			AudioCallbackNotifyCallbackReceivers();
			AudioCallbackClearOutPreviousCallBuffer(outputBuffer, bufferSampleCount);
			AudioCallbackProcessVoices(outputBuffer, bufferFrameCount, bufferSampleCount);
			AudioCallbackAdjustBufferMasterVolume(outputBuffer, bufferSampleCount);

			return AudioCallbackResult::Continue;
		}

		static int StaticAudioCallback(void* outputBuffer, void* userData, u32 bufferFrames, double streamTime, RtAudioStreamStatus, void*)
		{
			auto implInstance = static_cast<Impl*>(userData);
			return static_cast<int>(implInstance->AudioCallback(static_cast<i16*>(outputBuffer), bufferFrames, streamTime));
		}
	};

	AudioEngine::AudioEngine()
	{
		SetAudioAPI(AudioAPI::Default);

		impl->ActiveChannelMixer.SetTargetChannels(OutputChannelCount);
		impl->ActiveChannelMixer.SetMixingBehavior(ChannelMixer::MixingBehavior::Mix);

		// TODO: Uncomment once this is all working correctly
		// constexpr size_t initialCapacity = 64;
		// impl->LoadedSources.reserve(initialCapacity);
	}

	AudioEngine::~AudioEngine()
	{
		if (impl->IsStreamOpen)
			StopStream();
	}

	void AudioEngine::CreateInstance()
	{
		AudioEngineInstance = std::make_unique<AudioEngine>();
	}

	void AudioEngine::DeleteInstance()
	{
		AudioEngineInstance = nullptr;
	}

	AudioEngine& AudioEngine::GetInstance()
	{
		assert(AudioEngineInstance != nullptr);
		return *AudioEngineInstance;
	}

	void AudioEngine::OpenStream()
	{
		if (impl->IsStreamOpen)
			return;

		// TODO: Store user preference via string name
		const auto deviceID = impl->RtAudio.Context->getDefaultOutputDevice();

		RtAudio::StreamParameters streamParameters;
		streamParameters.deviceId = deviceID;
		streamParameters.nChannels = GetChannelCount();
		streamParameters.firstChannel = 0;

		auto& outputParameters = streamParameters;
		auto& inputParameters = streamParameters;

		auto format = RTAUDIO_SINT16;
		auto sampleRate = GetSampleRate();
		auto bufferFrames = impl->CurrentBufferFrameSize;
		void* userData = this;

#if 0
		try
		{
			impl->RtAudio.Context->openStream(&outputParameters, &inputParameters, format, sampleRate, &bufferFrames, &Impl::StaticAudioCallback, userData);
			impl->IsStreamOpen = true;
		}
		catch (const RtAudioError& exception)
		{
			Logger::LogErrorLine(__FUNCTION__"(): Failed: %s", exception.getMessage().c_str());
			return;
		}
#else
		impl->RtAudio.Context->openStream(&outputParameters, &inputParameters, format, sampleRate, &bufferFrames, &Impl::StaticAudioCallback, userData);
		impl->IsStreamOpen = true;
#endif
	}

	void AudioEngine::CloseStream()
	{
		if (!impl->IsStreamOpen)
			return;

		impl->IsStreamOpen = false;
		impl->IsStreamRunning = false;
		impl->RtAudio.Context->closeStream();
	}

	void AudioEngine::StartStream()
	{
		if (!impl->IsStreamOpen || impl->IsStreamRunning)
			return;

		impl->IsStreamRunning = true;
		impl->RtAudio.Context->startStream();
	}

	void AudioEngine::StopStream()
	{
		if (!impl->IsStreamOpen || !impl->IsStreamRunning)
			return;

		impl->IsStreamRunning = false;
		impl->RtAudio.Context->stopStream();
	}

	SourceHandle AudioEngine::LoadAudioSource(std::string_view filePath)
	{
		return LoadAudioSource(AudioDecoderFactory::GetInstance().DecodeFile(filePath));
	}

	SourceHandle AudioEngine::LoadAudioSource(std::unique_ptr<ISampleProvider> sampleProvider)
	{
		if (sampleProvider == nullptr)
			return SourceHandle::Invalid;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		for (size_t i = 0; i < impl->LoadedSources.size(); i++)
		{
			if (impl->LoadedSources[i] != nullptr)
				continue;

			impl->LoadedSources[i] = std::move(sampleProvider);
			return static_cast<SourceHandle>(i);
		}

		impl->LoadedSources.push_back(std::move(sampleProvider));
		return static_cast<SourceHandle>(impl->LoadedSources.size() - 1);
	}

	void AudioEngine::UnloadSource(SourceHandle source)
	{
		const auto lock = std::scoped_lock(impl->CallbackMutex);

		auto sourcePtr = IndexOrNull(static_cast<HandleBaseType>(source), impl->LoadedSources);
		if (sourcePtr == nullptr)
			return;

		*sourcePtr = nullptr;

		for (auto& voice : impl->ActiveVoices)
		{
			if ((voice.Flags & VoiceFlags_Alive) && voice.Source == source)
				voice.Source == SourceHandle::Invalid;
		}
	}

	VoiceHandle AudioEngine::AddVoice(SourceHandle source, std::string_view name, bool playing, f32 volume, bool playPastEnd)
	{
		const auto lock = std::scoped_lock(impl->CallbackMutex);

		for (size_t i = 0; i < impl->ActiveVoices.size(); i++)
		{
			auto& voiceToUpdate = impl->ActiveVoices[i];
			if (voiceToUpdate.Flags & VoiceFlags_Alive)
				continue;

			voiceToUpdate.Flags = VoiceFlags_Alive;
			if (playing) voiceToUpdate.Flags |= VoiceFlags_Playing;
			if (playPastEnd) voiceToUpdate.Flags |= VoiceFlags_PlayPastEnd;

			voiceToUpdate.Source = source;
			voiceToUpdate.Volume = volume;
			voiceToUpdate.FramePosition = 0;

			const auto adjustedNameLength = std::min(name.size() + 1, voiceToUpdate.Name.size()) - 1;
			std::copy(name.data(), name.data() + adjustedNameLength, voiceToUpdate.Name.data());
			voiceToUpdate.Name[adjustedNameLength] = '\0';

			return static_cast<VoiceHandle>(i);
		}

		return VoiceHandle::Invalid;
	}

	void AudioEngine::RemoveVoice(VoiceHandle voice)
	{
		// TODO: Think these locks through again
		const auto lock = std::scoped_lock(impl->CallbackMutex);

		if (auto voicePtr = IndexOrNull(static_cast<HandleBaseType>(voice), impl->ActiveVoices); voicePtr != nullptr)
			voicePtr->Flags = VoiceFlags_Dead;
	}

	void AudioEngine::RegisterCallbackReceiver(ICallbackReceiver* callbackReceiver)
	{
		for (auto& receiver : impl->RegisteredCallbackReceivers)
		{
			if (receiver != nullptr)
				continue;

			receiver = callbackReceiver;
			return;
		}

		impl->RegisteredCallbackReceivers.push_back(callbackReceiver);
	}

	void AudioEngine::UnregisterCallbackReceiver(ICallbackReceiver* callbackReceiver)
	{
		for (auto& receiver : impl->RegisteredCallbackReceivers)
		{
			if (receiver == callbackReceiver)
				receiver = nullptr;
		}
	}

	AudioEngine::AudioAPI AudioEngine::GetAudioAPI() const
	{
		return impl->CurrentAudioAPI;
	}

	void AudioEngine::SetAudioAPI(AudioAPI value)
	{
		impl->CurrentAudioAPI = value;
		const bool wasStreamRunning = (impl->IsStreamOpen && impl->IsStreamRunning);

		if (impl->IsStreamRunning)
			StopStream();

		if (impl->IsStreamOpen)
			CloseStream();

		impl->RtAudio.Context = std::make_unique<RtAudio>(impl->RtAudio.GetRtAudioAPI(value));

		if (wasStreamRunning)
		{
			OpenStream();
			StartStream();
		}
	}

	bool AudioEngine::GetIsStreamOpen() const
	{
		return impl->IsStreamOpen;
	}

	bool AudioEngine::GetIsStreamRunning() const
	{
		return impl->IsStreamRunning;
	}

	f32 AudioEngine::GetMasterVolume() const
	{
		return impl->MasterVolume;
	}

	void AudioEngine::SetMasterVolume(f32 value)
	{
		impl->MasterVolume = std::clamp(value, MinVolume, MaxVolume);
	}

	u32 AudioEngine::GetChannelCount() const
	{
		return OutputChannelCount;
	}

	u32 AudioEngine::GetSampleRate() const
	{
		return OutputSampleRate;
	}

	u32 AudioEngine::GetBufferFrameSize() const
	{
		return impl->CurrentBufferFrameSize;
	}

	void AudioEngine::SetBufferFrameSize(u32 bufferFrameSize)
	{
		bufferFrameSize = std::min(bufferFrameSize, MaxSampleBufferSize * OutputChannelCount);

		if (bufferFrameSize == impl->CurrentBufferFrameSize)
			return;

		impl->CurrentBufferFrameSize = bufferFrameSize;

		const bool wasStreamOpen = impl->IsStreamOpen;
		const bool wasStreamRunning = impl->IsStreamRunning;

		if (wasStreamOpen)
		{
			CloseStream();
			OpenStream();
		}

		if (wasStreamRunning && GetIsStreamOpen())
			StartStream();
	}

	TimeSpan AudioEngine::GetStreamTime() const
	{
		return impl->CallbackStreamTime;
	}

	void AudioEngine::SetStreamTime(TimeSpan value)
	{
		if (impl->IsStreamOpen)
		{
			impl->RtAudio.Context->setStreamTime(value.TotalSeconds());
			impl->CallbackStreamTime = value;
		}
	}

	bool AudioEngine::GetIsExclusiveMode() const
	{
		return impl->CurrentAudioAPI == AudioAPI::ASIO;
	}

	TimeSpan AudioEngine::GetCallbackLatency() const
	{
		return impl->CallbackLatency;
	}

	ChannelMixer& AudioEngine::GetChannelMixer()
	{
		return impl->ActiveChannelMixer;
	}

	void AudioEngine::DebugShowControlPanel() const
	{
		if (impl->CurrentAudioAPI != AudioAPI::ASIO)
			return;

		// NOTE: Defined in <rtaudio/asio.cpp>
		long ASIOControlPanel();

		ASIOControlPanel();
	}

	f32 Voice::GetVoiceVolume(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return voice->Volume;
		return 0.0f;
	}

	void Voice::SetVoiceVolume(VoiceHandle handle, f32 value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			voice->Volume = value;
	}

	TimeSpan Voice::GetVoicePosition(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
		{
			if (auto source = impl->GetSource(voice->Source); source != nullptr)
				return FramesToTimeSpan(voice->FramePosition, source->GetSampleRate());
		}
		return TimeSpan::Zero();
	}

	void Voice::SetVoicePosition(VoiceHandle handle, TimeSpan value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
		{
			if (auto source = impl->GetSource(voice->Source); source != nullptr)
				voice->FramePosition = TimeSpanToFrames(value, source->GetSampleRate());
		}
	}

	SourceHandle Voice::GetSource(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return voice->Source;
		return SourceHandle::Invalid;
	}

	void Voice::SetSource(VoiceHandle handle, SourceHandle value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			voice->Source = value;
	}

	TimeSpan Voice::GetVoiceDuration(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
		{
			if (auto source = impl->GetSource(voice->Source); source != nullptr)
				return FramesToTimeSpan(source->GetFrameCount(), source->GetSampleRate());
		}
		return TimeSpan::Zero();
	}

	bool Voice::GetVoiceIsPlaying(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return (voice->Flags & VoiceFlags_Playing);
		return false;
	}

	void Voice::SetVoiceIsPlaying(VoiceHandle handle, bool value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			voice->Flags = value ? (voice->Flags | VoiceFlags_Playing) : (voice->Flags & ~VoiceFlags_Playing);
	}

	bool Voice::GetVoiceIsLooping(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return (voice->Flags & VoiceFlags_Looping);
		return false;
	}

	void Voice::SetVoiceIsLooping(VoiceHandle handle, bool value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			voice->Flags = value ? (voice->Flags | VoiceFlags_Looping) : (voice->Flags & ~VoiceFlags_Looping);
	}

	bool Voice::GetVoicePlayPastEnd(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return (voice->Flags & VoiceFlags_PlayPastEnd);
		return false;
	}

	void Voice::SetVoicePlayPastEnd(VoiceHandle handle, bool value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			voice->Flags = value ? (voice->Flags | VoiceFlags_PlayPastEnd) : (voice->Flags & ~VoiceFlags_PlayPastEnd);
	}

	bool Voice::GetVoiceRemoveOnEnd(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return (voice->Flags & VoiceFlags_RemoveOnEnd);
		return false;
	}

	void Voice::SetVoiceRemoveOnEnd(VoiceHandle handle, bool value)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			voice->Flags = value ? (voice->Flags | VoiceFlags_RemoveOnEnd) : (voice->Flags & ~VoiceFlags_RemoveOnEnd);
	}

	std::string_view Voice::GetVoiceName(VoiceHandle handle)
	{
		auto& impl = AudioEngineInstance->impl;

		if (auto voice = impl->GetVoiceData(handle); voice != nullptr)
			return voice->Name.data();
		return "VoiceHandle::Invalid";
	}
}
