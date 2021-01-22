#include "AudioEngine.h"
#include "Resample.h"
#include "SampleMix.h"
#include "Backend/IAudioBackend.h"
#include "Backend/WASAPIBackend.h"
#include "Audio/Decoder/DecoderFactory.h"
#include "Audio/Decoder/Detail/Decoders.h"
#include "Core/Logger.h"
#include "Time/Stopwatch.h"
#include "IO/File.h"
#include "IO/Path.h"
#include <mutex>

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

		void CopyStringIntoBuffer(char* outputBuffer, const size_t bufferSize, const std::string_view stringToCopy)
		{
			const auto copyLength = std::min(stringToCopy.size() + 1, bufferSize) - 1;

			std::copy(stringToCopy.data(), stringToCopy.data() + copyLength, outputBuffer);
			outputBuffer[copyLength] = '\0';
		}

		std::unique_ptr<IAudioBackend> CreateBackendInterface(AudioBackend backend)
		{
			switch (backend)
			{
			case AudioBackend::WASAPIShared:
			case AudioBackend::WASAPIExclusive:
				return std::make_unique<WASAPIBackend>();
			}

			assert(false);
			return nullptr;
		}
	}

	std::unique_ptr<AudioEngine> EngineInstance = nullptr;

	using VoiceFlags = u16;
	enum VoiceFlagsEnum : VoiceFlags
	{
		VoiceFlags_Dead = 0,
		VoiceFlags_Alive = 1 << 0,
		VoiceFlags_Playing = 1 << 1,
		VoiceFlags_Looping = 1 << 2,
		VoiceFlags_PlayPastEnd = 1 << 3,
		VoiceFlags_RemoveOnEnd = 1 << 4,
		VoiceFlags_PauseOnEnd = 1 << 5,
		VoiceFlags_VariablePlaybackSpeed = 1 << 6,
	};

	struct AtomicVoiceVolumeMap
	{
		std::atomic<i64> StartFrame, EndFrame;
		std::atomic<f32> StartVolume, EndVolume;
	};

	// NOTE: Reusable voice instance internal data
	struct VoiceData
	{
		// NOTE: Automatically resets to SourceHandle::Invalid when the source is unloaded
		std::atomic<VoiceFlags> Flags;
		std::atomic<SourceHandle> Source;
		std::atomic<f32> Volume;
		std::atomic<i64> FramePosition;

		std::atomic<f32> PlaybackSpeed;
		std::atomic<f64> TimePositionSeconds;

		std::array<char, 64> Name;

		// TODO: Loop between
		// std::atomic<i64> LoopStartFrame, LoopEndFrame;

		AtomicVoiceVolumeMap VolumeMap;
	};

	struct AudioEngine::Impl
	{
	public:
		bool IsStreamOpenRunning = false;
		std::atomic<f32> MasterVolume = AudioEngine::MaxVolume;

	public:
		ChannelMixer ChannelMixer = {};

		AudioBackend CurrentBackendType = {};
		std::unique_ptr<IAudioBackend> CurrentBackend = nullptr;
		// TODO: Fallback backend interface

	public:
		std::mutex CallbackMutex;

		// NOTE: Indexed into by VoiceHandle, nullptr = free space
		std::array<VoiceData, MaxSimultaneousVoices> VoicePool;

		// NOTE: Indexed into by SourceHandle, nullptr = free space
		struct SourceData
		{
			SourceData() = default;
			SourceData(std::shared_ptr<ISampleProvider> provider, f32 volume, std::string name) : SampleProvider(std::move(provider)), BaseVolume(volume), Name(std::move(name)) {}
			SourceData(const SourceData& other) { *this = other; }
			SourceData(SourceData&& other) { *this = std::move(other); }
			~SourceData() = default;

			std::shared_ptr<ISampleProvider> SampleProvider = nullptr;
			std::atomic<f32> BaseVolume = 0.0f;
			std::string Name;

			SourceData& operator=(const SourceData& other) { SampleProvider = other.SampleProvider; BaseVolume = other.BaseVolume.load(); Name = other.Name; return *this; }
			SourceData& operator=(SourceData&& other) { SampleProvider = std::move(other.SampleProvider); BaseVolume = other.BaseVolume.load(); Name = std::move(other.Name); return *this; }
		};

		std::vector<SourceData> LoadedSources;

	public:
		static constexpr size_t TempOutputBufferSize = (MaxBufferSampleCount * OutputChannelCount);

		std::array<i16, TempOutputBufferSize> TempOutputBuffer = {};
		u32 CurrentBufferFrameSize = DefaultBufferFrameCount;

		// NOTE: For measuring performance
		size_t CallbackDurationRingIndex = 0;
		std::array<TimeSpan, AudioEngine::CallbackDurationRingBufferSize> CallbackDurationsRingBuffer = {};

		// NOTE: For visualizing the current audio output
		size_t LastPlayedSamplesRingIndex = 0;
		std::array<std::array<i16, AudioEngine::LastPlayedSamplesRingBufferFrameCount>, OutputChannelCount> LastPlayedSamplesRingBuffer = {};

		Stopwatch StreamTimeStopwatch = {};

		TimeSpan CallbackFrequency = {};
		TimeSpan CallbackStreamTime = {}, LastCallbackStreamTime = {};

		// NOTE: nullptr = free space
		std::vector<CallbackReceiver*> RegisteredCallbackReceivers;

	public:
		struct DebugCaptureData
		{
			bool RecordOutput = false;
			std::mutex Mutex;
			std::vector<i16> RecordedSamples;

			void DumpSamplesToWaveFile(std::string_view filePath)
			{
				const auto[fileContent, fileSize] = WavDecoder::Encode(RecordedSamples.data(), RecordedSamples.size(), OutputSampleRate, OutputChannelCount);
				if (fileContent == nullptr)
					return;

				IO::File::WriteAllBytes(filePath, fileContent.get(), fileSize);
			}
		} DebugCapture;

	public:
		VoiceData* GetVoiceData(VoiceHandle handle)
		{
			auto voice = IndexOrNull(static_cast<HandleBaseType>(handle), VoicePool);
			return (voice != nullptr && (voice->Flags & VoiceFlags_Alive)) ? voice : nullptr;
		}

		ISampleProvider* GetSource(SourceHandle source)
		{
			auto sourceData = IndexOrNull(static_cast<HandleBaseType>(source), LoadedSources);
			return (sourceData != nullptr && sourceData->SampleProvider != nullptr) ? sourceData->SampleProvider.get() : nullptr;
		}

		std::shared_ptr<ISampleProvider> GetSharedSource(SourceHandle source)
		{
			auto sourceData = IndexOrNull(static_cast<HandleBaseType>(source), LoadedSources);
			return (sourceData != nullptr && sourceData->SampleProvider != nullptr) ? sourceData->SampleProvider : nullptr;
		}

		f32 GetSourceBaseVolume(SourceHandle source)
		{
			auto sourceData = IndexOrNull(static_cast<HandleBaseType>(source), LoadedSources);
			return (sourceData != nullptr && sourceData->SampleProvider != nullptr) ? sourceData->BaseVolume.load() : 1.0f;
		}

		void SetSourceBaseVolume(SourceHandle source, f32 value)
		{
			auto sourceData = IndexOrNull(static_cast<HandleBaseType>(source), LoadedSources);
			if (sourceData != nullptr && sourceData->SampleProvider != nullptr)
				sourceData->BaseVolume.store(value);
		}

		void GetSourceName(SourceHandle source, std::string& outName)
		{
			auto sourceData = IndexOrNull(static_cast<HandleBaseType>(source), LoadedSources);
			if (sourceData != nullptr && sourceData->SampleProvider != nullptr)
				outName = sourceData->Name;
		}

		void SetSourceName(SourceHandle source, std::string_view newName)
		{
			auto sourceData = IndexOrNull(static_cast<HandleBaseType>(source), LoadedSources);
			if (sourceData != nullptr && sourceData->SampleProvider != nullptr)
				sourceData->Name = newName;
		}

		void CallbackNotifyCallbackReceivers()
		{
			for (auto* callbackReceiver : RegisteredCallbackReceivers)
			{
				if (callbackReceiver != nullptr)
					callbackReceiver->OnAudioCallback();
			}
		}

		void CallbackClearOutPreviousCallBuffer(i16* outputBuffer, const size_t sampleCount)
		{
			std::fill(outputBuffer, outputBuffer + sampleCount, 0);
		}

		f32 SampleVolumeMapAt(const i64 startFrame, const i64 endFrame, const f32 startVolume, const f32 endVolume, const i64 frame)
		{
			if (frame <= startFrame)
				return startVolume;

			if (frame >= endFrame)
				return endVolume;

			const f64 delta = static_cast<f64>(frame - startFrame) / static_cast<f64>(endFrame - startFrame);
			const f32 lerpVolume = (startVolume + (endVolume - startVolume) * static_cast<f32>(delta));

			return lerpVolume;
		}

		void CallbackApplyVoiceVolumeAndMixTempBufferIntoOutput(i16* outputBuffer, const i64 frameCount, const VoiceData& voiceData, const u32 sampleRate)
		{
			const f32 voiceVolume = voiceData.Volume * GetSourceBaseVolume(voiceData.Source);
			const f32 startVolume = voiceData.VolumeMap.StartVolume;
			const f32 endVolume = voiceData.VolumeMap.EndVolume;

			if (startVolume == endVolume)
			{
				for (i64 i = 0; i < (frameCount * OutputChannelCount); i++)
					outputBuffer[i] = MixSamples(outputBuffer[i], static_cast<i16>(TempOutputBuffer[i] * voiceVolume));
			}
			else
			{
				const i64 volumeMapStartFrame = voiceData.VolumeMap.StartFrame;
				const i64 volumeMapEndFrame = voiceData.VolumeMap.EndFrame;

				if (voiceData.Flags & VoiceFlags_VariablePlaybackSpeed)
				{
					const auto frameDuration = FramesToTimeSpan(1, sampleRate) * voiceData.PlaybackSpeed;
					const auto bufferDuration = TimeSpan::FromSeconds(frameDuration.TotalSeconds() * frameCount);
					const auto voiceStartTime = TimeSpan::FromSeconds(voiceData.TimePositionSeconds) - bufferDuration;

					for (i64 f = 0; f < frameCount; f++)
					{
						const auto frameTime = TimeSpan::FromSeconds(voiceStartTime.TotalSeconds() + (f * frameDuration.TotalSeconds()));
						const f32 frameVolume = SampleVolumeMapAt(volumeMapStartFrame, volumeMapEndFrame, startVolume, endVolume, TimeSpanToFrames(frameTime, sampleRate)) * voiceVolume;

						for (u32 c = 0; c < OutputChannelCount; c++)
						{
							const auto sampleIndex = (f * OutputChannelCount) + c;
							outputBuffer[sampleIndex] = MixSamples(outputBuffer[sampleIndex], static_cast<i16>(TempOutputBuffer[sampleIndex] * frameVolume));
						}
					}
				}
				else
				{
					const i64 voiceStartFrame = (voiceData.FramePosition - frameCount);

					for (i64 f = 0; f < frameCount; f++)
					{
						const f32 frameVolume = SampleVolumeMapAt(volumeMapStartFrame, volumeMapEndFrame, startVolume, endVolume, voiceStartFrame + f) * voiceVolume;
						for (u32 c = 0; c < OutputChannelCount; c++)
						{
							const auto sampleIndex = (f * OutputChannelCount) + c;
							outputBuffer[sampleIndex] = MixSamples(outputBuffer[sampleIndex], static_cast<i16>(TempOutputBuffer[sampleIndex] * frameVolume));
						}
					}
				}
			}
		}

		void CallbackProcessVoices(i16* outputBuffer, const u32 bufferFrameCount, const u32 bufferSampleCount)
		{
			const auto lock = std::scoped_lock(CallbackMutex);

			for (auto& voiceData : VoicePool)
			{
				if (!(voiceData.Flags & VoiceFlags_Alive))
					continue;

				auto* sampleProvider = GetSource(voiceData.Source);

				const bool variablePlaybackSpeed = (voiceData.Flags & VoiceFlags_VariablePlaybackSpeed);
				const bool playPastEnd = (voiceData.Flags & VoiceFlags_PlayPastEnd);
				bool hasReachedEnd = (sampleProvider == nullptr) ? false :
					(variablePlaybackSpeed ? (voiceData.TimePositionSeconds >= FramesToTimeSpan(sampleProvider->GetFrameCount(), sampleProvider->GetSampleRate()).TotalSeconds()) :
					(voiceData.FramePosition >= sampleProvider->GetFrameCount()));

				if (sampleProvider == nullptr && (voiceData.Flags & VoiceFlags_RemoveOnEnd))
					hasReachedEnd = true;

				if (hasReachedEnd)
				{
					if (!playPastEnd && (voiceData.Flags & VoiceFlags_RemoveOnEnd))
					{
						voiceData.Flags = VoiceFlags_Dead;
						continue;
					}
					else if (voiceData.Flags & VoiceFlags_PauseOnEnd)
					{
						voiceData.Flags &= ~VoiceFlags_Playing;
						continue;
					}
				}

				if (voiceData.Flags & VoiceFlags_Playing)
				{
					if (variablePlaybackSpeed)
						CallbackProcessVariableSpeedVoiceSamples(outputBuffer, bufferFrameCount, playPastEnd, hasReachedEnd, voiceData, sampleProvider);
					else
						CallbackProcessNormalSpeedVoiceSamples(outputBuffer, bufferFrameCount, playPastEnd, hasReachedEnd, voiceData, sampleProvider);
				}
			}
		}

		void CallbackProcessNormalSpeedVoiceSamples(i16* outputBuffer, const u32 bufferFrameCount, const bool playPastEnd, const bool hasReachedEnd, VoiceData& voiceData, ISampleProvider* sampleProvider)
		{
			if (sampleProvider == nullptr)
			{
				voiceData.FramePosition += bufferFrameCount;
				return;
			}

			i64 framesRead = 0;
			if (sampleProvider->GetChannelCount() != OutputChannelCount)
				framesRead = ChannelMixer.MixChannels(*sampleProvider, TempOutputBuffer.data(), voiceData.FramePosition, bufferFrameCount);
			else
				framesRead = sampleProvider->ReadSamples(TempOutputBuffer.data(), voiceData.FramePosition, bufferFrameCount);
			voiceData.FramePosition += framesRead;

			if (hasReachedEnd && !playPastEnd)
				voiceData.FramePosition = (voiceData.Flags & VoiceFlags_Looping) ? 0 : sampleProvider->GetFrameCount();

			CallbackApplyVoiceVolumeAndMixTempBufferIntoOutput(outputBuffer, framesRead, voiceData, sampleProvider->GetSampleRate());
		}

		void CallbackProcessVariableSpeedVoiceSamples(i16* outputBuffer, const u32 bufferFrameCount, const bool playPastEnd, const bool hasReachedEnd, VoiceData& voiceData, ISampleProvider* sampleProvider)
		{
			const auto sampleRate = (sampleProvider != nullptr) ? sampleProvider->GetSampleRate() : OutputSampleRate;
			const auto bufferDurationSec = (FramesToTimeSpan(bufferFrameCount, sampleRate).TotalSeconds() * voiceData.PlaybackSpeed);

			// TODO: Implement without the need for a raw sample view if it's ever needed
			const auto rawSamples = (sampleProvider != nullptr) ? sampleProvider->GetRawSampleView() : nullptr;

			if (sampleProvider == nullptr || rawSamples == nullptr)
			{
				voiceData.TimePositionSeconds = voiceData.TimePositionSeconds + bufferDurationSec;
				return;
			}

			const f64 sampleDurationSec = (1.0 / static_cast<i64>(sampleRate)) * voiceData.PlaybackSpeed;
			const i64 framesRead = static_cast<i64>(glm::round(bufferDurationSec / sampleDurationSec));

			const size_t providerSampleCount = sampleProvider->GetFrameCount() * sampleProvider->GetChannelCount();
			const u32 providerChannelCount = sampleProvider->GetChannelCount();

			const f64 sampleRateF64 = static_cast<f64>(sampleRate);
			const f64 voiceStartTimeSec = voiceData.TimePositionSeconds;

			if (providerChannelCount != OutputChannelCount)
			{
				auto mixBuffer = ChannelMixer.GetMixSampleBuffer(framesRead * providerChannelCount);

				for (i64 f = 0; f < framesRead; f++)
				{
					const auto frameTimeSec = voiceStartTimeSec + (f * sampleDurationSec);
					for (u32 c = 0; c < providerChannelCount; c++)
						mixBuffer[(f * providerChannelCount) + c] = SampleAtTime<i16, Interpolation::Linear>(frameTimeSec, c, rawSamples, providerSampleCount, sampleRateF64, providerChannelCount);
				}

				ChannelMixer.MixChannels(providerChannelCount, mixBuffer, framesRead, TempOutputBuffer.data(), 0, framesRead);
			}
			else
			{
				for (i64 f = 0; f < framesRead; f++)
				{
					const auto frameTimeSec = voiceStartTimeSec + (f * sampleDurationSec);
					for (u32 c = 0; c < OutputChannelCount; c++)
						TempOutputBuffer[(f * OutputChannelCount) + c] = SampleAtTime<i16, Interpolation::Linear>(frameTimeSec, c, rawSamples, providerSampleCount, sampleRateF64, OutputChannelCount);
				}
			}

			voiceData.TimePositionSeconds = voiceData.TimePositionSeconds + bufferDurationSec;
			if (hasReachedEnd && !playPastEnd)
				voiceData.TimePositionSeconds = (voiceData.Flags & VoiceFlags_Looping) ? 0.0 : FramesToTimeSpan(sampleProvider->GetFrameCount(), sampleRate).TotalSeconds();

			CallbackApplyVoiceVolumeAndMixTempBufferIntoOutput(outputBuffer, framesRead, voiceData, sampleRate);
		}

		void CallbackAdjustBufferMasterVolume(i16* outputBuffer, const size_t sampleCount)
		{
			const auto masterVolume = MasterVolume.load();
			for (auto i = 0; i < sampleCount; i++)
				outputBuffer[i] = static_cast<i16>(outputBuffer[i] * masterVolume);
		}

		void CallbackDebugRecordOutput(i16* outputBuffer, const size_t sampleCount)
		{
			if (!DebugCapture.RecordOutput)
				return;

			const auto lock = std::scoped_lock(DebugCapture.Mutex);
			for (size_t i = 0; i < sampleCount; i++)
				DebugCapture.RecordedSamples.push_back(outputBuffer[i]);
		}

		void CallbackUpdateLastPlayedSamplesRingBuffer(i16* outputBuffer, const size_t frameCount)
		{
			for (size_t f = 0; f < frameCount; f++)
			{
				for (u32 c = 0; c < OutputChannelCount; c++)
					LastPlayedSamplesRingBuffer[c][LastPlayedSamplesRingIndex] = outputBuffer[(f * OutputChannelCount) + c];

				if (LastPlayedSamplesRingIndex++ >= (LastPlayedSamplesRingBuffer[0].size() - 1))
					LastPlayedSamplesRingIndex = 0;
			}
		}

		void CallbackUpdateCallbackDurationRingBuffer(const TimeSpan duration)
		{
			CallbackDurationsRingBuffer[CallbackDurationRingIndex] = duration;
			if (CallbackDurationRingIndex++ >= (CallbackDurationsRingBuffer.size() - 1))
				CallbackDurationRingIndex = 0;
		}

		void RenderAudioCallback(i16* outputBuffer, const u32 bufferFrameCount, const u32 bufferChannelCount)
		{
			auto stopwatch = Stopwatch::StartNew();;

			const auto bufferSampleCount = (bufferFrameCount * OutputChannelCount);
			CurrentBufferFrameSize = bufferFrameCount;

			CallbackStreamTime = StreamTimeStopwatch.GetElapsed();
			CallbackFrequency = (CallbackStreamTime - LastCallbackStreamTime);
			LastCallbackStreamTime = CallbackStreamTime;

			CallbackNotifyCallbackReceivers();
			CallbackClearOutPreviousCallBuffer(outputBuffer, bufferSampleCount);
			CallbackProcessVoices(outputBuffer, bufferFrameCount, bufferSampleCount);
			CallbackAdjustBufferMasterVolume(outputBuffer, bufferSampleCount);
			CallbackDebugRecordOutput(outputBuffer, bufferSampleCount);
			CallbackUpdateLastPlayedSamplesRingBuffer(outputBuffer, bufferFrameCount);
			CallbackUpdateCallbackDurationRingBuffer(stopwatch.Stop());
		}
	};

	AudioEngine::AudioEngine() : impl(std::make_unique<Impl>())
	{
		SetAudioBackend(AudioBackend::Default);

		impl->ChannelMixer.SetTargetChannels(OutputChannelCount);
		impl->ChannelMixer.SetMixingBehavior(ChannelMixer::MixingBehavior::Combine);

		constexpr size_t reasonableInitialSourceCapacity = 256;
		impl->LoadedSources.reserve(reasonableInitialSourceCapacity);

		constexpr size_t reasonableInitialCallbackReceiverCapacity = 4;
		impl->RegisteredCallbackReceivers.reserve(reasonableInitialCallbackReceiverCapacity);
	}

	AudioEngine::~AudioEngine()
	{
		StopCloseStream();
	}

	void AudioEngine::CreateInstance()
	{
		EngineInstance = std::make_unique<AudioEngine>();
	}

	void AudioEngine::DeleteInstance()
	{
		EngineInstance = nullptr;
	}

	bool AudioEngine::InstanceValid()
	{
		return (EngineInstance != nullptr);
	}

	AudioEngine& AudioEngine::GetInstance()
	{
		assert(EngineInstance != nullptr);
		return *EngineInstance;
	}

	void AudioEngine::OpenStartStream()
	{
		if (impl->IsStreamOpenRunning)
			return;

		StreamParameters streamParam = {};
		streamParam.SampleRate = OutputSampleRate;
		streamParam.ChannelCount = OutputChannelCount;
		streamParam.DesiredFrameCount = impl->CurrentBufferFrameSize;
		streamParam.Mode = (impl->CurrentBackendType == AudioBackend::WASAPIExclusive) ? StreamShareMode::Exclusive : StreamShareMode::Shared;

		if (impl->CurrentBackend == nullptr)
			impl->CurrentBackend = CreateBackendInterface(impl->CurrentBackendType);

		if (impl->CurrentBackend == nullptr)
			return;

		const bool openStreamSuccess = impl->CurrentBackend->OpenStartStream(streamParam, [this](i16* outputBuffer, const u32 bufferFrameCount, const u32 bufferChannelCount)
		{
			impl->RenderAudioCallback(outputBuffer, bufferFrameCount, bufferChannelCount);
		});

		if (openStreamSuccess)
			impl->StreamTimeStopwatch.Restart();

		impl->IsStreamOpenRunning = openStreamSuccess;
	}

	void AudioEngine::StopCloseStream()
	{
		if (!impl->IsStreamOpenRunning)
			return;

		if (impl->CurrentBackend != nullptr)
			impl->CurrentBackend->StopCloseStream();

		impl->StreamTimeStopwatch.Stop();

		impl->IsStreamOpenRunning = false;
	}

	void AudioEngine::EnsureStreamRunning()
	{
		if (GetIsStreamOpenRunning())
			return;

		OpenStartStream();

		const bool exclusiveAndFailedToStart = (impl->CurrentBackendType == AudioBackend::WASAPIExclusive && !GetIsStreamOpenRunning());
		if (exclusiveAndFailedToStart)
		{
			// NOTE: Because *any* audio is probably always better than *no* audio
			SetAudioBackend(AudioBackend::WASAPIShared);
			OpenStartStream();
		}
	}

	std::future<SourceHandle> AudioEngine::LoadSourceAsync(std::string_view filePath)
	{
		return std::async(std::launch::async, [this, path = std::string(filePath)]()
		{
			return LoadSource(path);
		});
	}

	SourceHandle AudioEngine::LoadSource(std::string_view filePath)
	{
		return RegisterSource(DecoderFactory::GetInstance().DecodeFile(filePath), IO::Path::GetFileName(filePath));
	}

	SourceHandle AudioEngine::LoadSource(std::string_view fileName, const void* fileContent, size_t fileSize)
	{
		return RegisterSource(DecoderFactory::GetInstance().DecodeFile(fileName, fileContent, fileSize), fileName);
	}

	SourceHandle AudioEngine::RegisterSource(std::shared_ptr<ISampleProvider> sampleProvider, std::string_view name)
	{
		if (sampleProvider == nullptr)
			return SourceHandle::Invalid;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		for (size_t i = 0; i < impl->LoadedSources.size(); i++)
		{
			auto& sourceData = impl->LoadedSources[i];
			if (sourceData.SampleProvider != nullptr)
				continue;

			sourceData.SampleProvider = std::move(sampleProvider);
			sourceData.BaseVolume = 1.0f;
			sourceData.Name = name;

			return static_cast<SourceHandle>(i);
		}

		impl->LoadedSources.push_back(Impl::SourceData { std::move(sampleProvider), 1.0f, std::string(name) });
		return static_cast<SourceHandle>(impl->LoadedSources.size() - 1);
	}

	void AudioEngine::UnloadSource(SourceHandle source)
	{
		if (source == SourceHandle::Invalid)
			return;

		const auto lock = std::scoped_lock(impl->CallbackMutex);

		auto sourcePtr = IndexOrNull(static_cast<HandleBaseType>(source), impl->LoadedSources);
		if (sourcePtr == nullptr)
			return;

		sourcePtr->SampleProvider = nullptr;

		for (auto& voice : impl->VoicePool)
		{
			if ((voice.Flags & VoiceFlags_Alive) && voice.Source == source)
				voice.Source = SourceHandle::Invalid;
		}
	}

	VoiceHandle AudioEngine::AddVoice(SourceHandle source, std::string_view name, bool playing, f32 volume, bool playPastEnd)
	{
		const auto lock = std::scoped_lock(impl->CallbackMutex);

		for (size_t i = 0; i < impl->VoicePool.size(); i++)
		{
			auto& voiceToUpdate = impl->VoicePool[i];
			if (voiceToUpdate.Flags & VoiceFlags_Alive)
				continue;

			voiceToUpdate.Flags = VoiceFlags_Alive;
			if (playing) voiceToUpdate.Flags |= VoiceFlags_Playing;
			if (playPastEnd) voiceToUpdate.Flags |= VoiceFlags_PlayPastEnd;

			voiceToUpdate.Source = source;
			voiceToUpdate.Volume = volume;
			voiceToUpdate.FramePosition = 0;
			voiceToUpdate.VolumeMap.StartVolume = 0.0f;
			voiceToUpdate.VolumeMap.EndVolume = 0.0f;
			CopyStringIntoBuffer(voiceToUpdate.Name.data(), voiceToUpdate.Name.size(), name);

			return static_cast<VoiceHandle>(i);
		}

#if COMFY_DEBUG // DEBUG: Consider increasing MaxSimultaneousVoices...
		assert(false);
#endif

		return VoiceHandle::Invalid;
	}

	void AudioEngine::RemoveVoice(VoiceHandle voice)
	{
		// TODO: Think these locks through again
		const auto lock = std::scoped_lock(impl->CallbackMutex);

		if (auto voicePtr = IndexOrNull(static_cast<HandleBaseType>(voice), impl->VoicePool); voicePtr != nullptr)
			voicePtr->Flags = VoiceFlags_Dead;
	}

	void AudioEngine::PlayOneShotSound(SourceHandle source, std::string_view name, f32 volume)
	{
		if (source == SourceHandle::Invalid)
			return;

		const auto lock = std::scoped_lock(impl->CallbackMutex);

		for (auto& voiceToUpdate : impl->VoicePool)
		{
			if (voiceToUpdate.Flags & VoiceFlags_Alive)
				continue;

			voiceToUpdate.Flags = VoiceFlags_Alive | VoiceFlags_Playing | VoiceFlags_RemoveOnEnd;
			voiceToUpdate.Source = source;
			voiceToUpdate.Volume = volume;
			voiceToUpdate.FramePosition = 0;
			voiceToUpdate.VolumeMap.StartVolume = 0.0f;
			voiceToUpdate.VolumeMap.EndVolume = 0.0f;
			CopyStringIntoBuffer(voiceToUpdate.Name.data(), voiceToUpdate.Name.size(), name);
			return;
		}
	}

	std::shared_ptr<ISampleProvider> AudioEngine::GetSharedSource(SourceHandle source)
	{
		if (source == SourceHandle::Invalid)
			nullptr;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		return impl->GetSharedSource(source);
	}

	f32 AudioEngine::GetSourceBaseVolume(SourceHandle source)
	{
		if (source == SourceHandle::Invalid)
			nullptr;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		return impl->GetSourceBaseVolume(source);
	}

	void AudioEngine::SetSourceBaseVolume(SourceHandle source, f32 value)
	{
		if (source == SourceHandle::Invalid)
			nullptr;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		return impl->SetSourceBaseVolume(source, value);
	}

	void AudioEngine::GetSourceName(SourceHandle source, std::string& outName)
	{
		if (source == SourceHandle::Invalid)
			nullptr;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		return impl->GetSourceName(source, outName);
	}

	void AudioEngine::SetSourceName(SourceHandle source, std::string_view newName)
	{
		if (source == SourceHandle::Invalid)
			nullptr;

		const auto lock = std::scoped_lock(impl->CallbackMutex);
		return impl->SetSourceName(source, newName);
	}

	AudioBackend AudioEngine::GetAudioBackend() const
	{
		return impl->CurrentBackendType;
	}

	void AudioEngine::SetAudioBackend(AudioBackend value)
	{
		if (value == impl->CurrentBackendType)
			return;

		impl->CurrentBackendType = value;
		if (impl->IsStreamOpenRunning)
		{
			StopCloseStream();
			impl->CurrentBackend = CreateBackendInterface(value);
			OpenStartStream();
		}
		else
		{
			impl->CurrentBackend = CreateBackendInterface(value);
		}
	}

	bool AudioEngine::GetIsStreamOpenRunning() const
	{
		return impl->IsStreamOpenRunning;
	}

	bool AudioEngine::GetAllVoicesAreIdle() const
	{
		if (!impl->IsStreamOpenRunning)
			return true;

		for (size_t i = 0; i < impl->VoicePool.size(); i++)
		{
			const auto& voice = impl->VoicePool[i];
			if ((voice.Flags & VoiceFlags_Alive) && (voice.Flags & VoiceFlags_Playing))
				return false;
		}

		return true;
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

	void AudioEngine::SetBufferFrameSize(u32 bufferFrameCount)
	{
		bufferFrameCount = std::clamp(bufferFrameCount, MinBufferFrameCount, MaxBufferFrameCount);

		if (bufferFrameCount == impl->CurrentBufferFrameSize)
			return;

		impl->CurrentBufferFrameSize = bufferFrameCount;

		if (GetIsStreamOpenRunning())
		{
			StopCloseStream();
			OpenStartStream();
		}
	}

	TimeSpan AudioEngine::GetCallbackFrequency() const
	{
		return impl->CallbackFrequency;
	}

	ChannelMixer& AudioEngine::GetChannelMixer()
	{
		return impl->ChannelMixer;
	}

	void AudioEngine::DebugShowControlPanel() const
	{
#if 0 // TODO: Implement together with ASIO backend
		if (impl->CurrentBackendType != AudioBackend::ASIO)
			return;

		long ASIOControlPanel();
		ASIOControlPanel();
#endif
	}

	void AudioEngine::DebugGetAllVoices(Voice* outputVoices, size_t* outputVoiceCount)
	{
		assert(outputVoices != nullptr && outputVoiceCount != nullptr);
		size_t voiceCount = 0;

		for (size_t i = 0; i < impl->VoicePool.size(); i++)
		{
			const auto& voice = impl->VoicePool[i];

			if (voice.Flags & VoiceFlags_Alive)
				outputVoices[voiceCount++] = static_cast<VoiceHandle>(i);
		}

		*outputVoiceCount = voiceCount;
	}

	size_t AudioEngine::DebugGetMaxSourceCount()
	{
		const auto lock = std::scoped_lock(impl->DebugCapture.Mutex);
		return impl->LoadedSources.size();
	}

	std::array<TimeSpan, AudioEngine::CallbackDurationRingBufferSize> AudioEngine::DebugGetCallbackDurations()
	{
		return impl->CallbackDurationsRingBuffer;
	}

	std::array<std::array<i16, AudioEngine::LastPlayedSamplesRingBufferFrameCount>, AudioEngine::OutputChannelCount> AudioEngine::DebugGetLastPlayedSamples()
	{
		return impl->LastPlayedSamplesRingBuffer;
	}

	bool AudioEngine::DebugGetEnableOutputCapture() const
	{
		return impl->DebugCapture.RecordOutput;
	}

	void AudioEngine::DebugSetEnableOutputCapture(bool value)
	{
		if (value && !impl->DebugCapture.RecordOutput)
		{
			constexpr size_t reasonableInitialCapacity = (OutputSampleRate * OutputChannelCount * 30);
			if (impl->DebugCapture.RecordedSamples.capacity() < reasonableInitialCapacity)
				impl->DebugCapture.RecordedSamples.reserve(reasonableInitialCapacity);
		}

		impl->DebugCapture.RecordOutput = value;
	}

	void AudioEngine::DebugFlushCaptureDiscard()
	{
		const auto lock = std::scoped_lock(impl->DebugCapture.Mutex);
		impl->DebugCapture.RecordedSamples.clear();
	}

	void AudioEngine::DebugFlushCaptureToWaveFile(std::string_view filePath)
	{
		const auto lock = std::scoped_lock(impl->DebugCapture.Mutex);
		impl->DebugCapture.DumpSamplesToWaveFile(filePath);
		impl->DebugCapture.RecordedSamples.clear();
	}

	bool Voice::IsValid() const
	{
		auto& impl = EngineInstance->impl;
		return (impl->GetVoiceData(Handle) != nullptr);
	}

	f32 Voice::GetVolume() const
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
			return voice->Volume;
		return 0.0f;
	}

	void Voice::SetVolume(f32 value)
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
			voice->Volume = value;
	}

	f32 Voice::GetPlaybackSpeed() const
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
		{
			if (voice->Flags & VoiceFlags_VariablePlaybackSpeed)
				return voice->PlaybackSpeed;
		}
		return 1.0f;
	}

	void Voice::SetPlaybackSpeed(f32 value)
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
		{
			const auto source = impl->GetSource(voice->Source);
			const auto sampleRate = (source != nullptr) ? source->GetSampleRate() : AudioEngine::OutputSampleRate;

			if (value == 1.0f)
			{
				if (voice->Flags & VoiceFlags_VariablePlaybackSpeed)
					voice->FramePosition = TimeSpanToFrames(TimeSpan::FromSeconds(voice->TimePositionSeconds), sampleRate);

				voice->Flags &= ~VoiceFlags_VariablePlaybackSpeed;
			}
			else
			{
				if (!(voice->Flags & VoiceFlags_VariablePlaybackSpeed))
					voice->TimePositionSeconds = FramesToTimeSpan(voice->FramePosition, sampleRate).TotalSeconds();

				voice->Flags |= VoiceFlags_VariablePlaybackSpeed;
			}

			voice->PlaybackSpeed = value;
		}
	}

	TimeSpan Voice::GetPosition() const
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
		{
			const auto source = impl->GetSource(voice->Source);
			const auto sampleRate = (source != nullptr) ? source->GetSampleRate() : AudioEngine::OutputSampleRate;

			if (voice->Flags & VoiceFlags_VariablePlaybackSpeed)
				return TimeSpan::FromSeconds(voice->TimePositionSeconds);
			else
				return FramesToTimeSpan(voice->FramePosition, sampleRate);
		}
		return TimeSpan::Zero();
	}

	void Voice::SetPosition(TimeSpan value)
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
		{
			const auto source = impl->GetSource(voice->Source);
			const auto sampleRate = (source != nullptr) ? source->GetSampleRate() : AudioEngine::OutputSampleRate;
			voice->FramePosition = TimeSpanToFrames(value, sampleRate);
			voice->TimePositionSeconds = value.TotalSeconds();
		}
	}

	SourceHandle Voice::GetSource() const
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
			return voice->Source;
		return SourceHandle::Invalid;
	}

	void Voice::SetSource(SourceHandle value)
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
			voice->Source = value;
	}

	TimeSpan Voice::GetDuration() const
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
		{
			if (auto source = impl->GetSource(voice->Source); source != nullptr)
				return FramesToTimeSpan(source->GetFrameCount(), source->GetSampleRate());
		}
		return TimeSpan::Zero();
	}

	bool Voice::GetIsPlaying() const
	{
		return GetInternalFlag(VoiceFlags_Playing);
	}

	void Voice::SetIsPlaying(bool value)
	{
		SetInternalFlag(VoiceFlags_Playing, value);
	}

	bool Voice::GetIsLooping() const
	{
		return GetInternalFlag(VoiceFlags_Looping);
	}

	void Voice::SetIsLooping(bool value)
	{
		SetInternalFlag(VoiceFlags_Looping, value);
	}

	bool Voice::GetPlayPastEnd() const
	{
		return GetInternalFlag(VoiceFlags_PlayPastEnd);
	}

	void Voice::SetPlayPastEnd(bool value)
	{
		SetInternalFlag(VoiceFlags_PlayPastEnd, value);
	}

	bool Voice::GetRemoveOnEnd() const
	{
		return GetInternalFlag(VoiceFlags_RemoveOnEnd);
	}

	void Voice::SetRemoveOnEnd(bool value)
	{
		SetInternalFlag(VoiceFlags_RemoveOnEnd, value);
	}

	bool Voice::GetPauseOnEnd() const
	{
		return GetInternalFlag(VoiceFlags_PauseOnEnd);
	}

	void Voice::SetPauseOnEnd(bool value)
	{
		SetInternalFlag(VoiceFlags_PauseOnEnd, value);
	}

	std::string_view Voice::GetName() const
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
			return voice->Name.data();
		return "VoiceHandle::Invalid";
	}

	void Voice::ResetVolumeMap()
	{
		if (auto voice = EngineInstance->impl->GetVoiceData(Handle); voice != nullptr)
		{
			voice->VolumeMap.StartFrame = 0;
			voice->VolumeMap.EndFrame = 0;
			voice->VolumeMap.StartVolume = 0.0f;
			voice->VolumeMap.EndVolume = 0.0f;
		}
	}

	void Voice::SetVolumeMap(TimeSpan startTime, TimeSpan endTime, f32 startVolume, f32 endVolume)
	{
		auto& impl = EngineInstance->impl;

		if (auto voice = impl->GetVoiceData(Handle); voice != nullptr)
		{
			const auto source = impl->GetSource(voice->Source);
			const auto sampleRate = (source != nullptr) ? source->GetSampleRate() : AudioEngine::OutputSampleRate;

			voice->VolumeMap.StartFrame = TimeSpanToFrames(startTime, sampleRate);
			voice->VolumeMap.EndFrame = TimeSpanToFrames(endTime, sampleRate);
			voice->VolumeMap.StartVolume = startVolume;
			voice->VolumeMap.EndVolume = endVolume;
		}
	}

	bool Voice::GetInternalFlag(u16 flag) const
	{
		static_assert(sizeof(flag) == sizeof(VoiceFlags));
		const auto voiceFlag = static_cast<VoiceFlags>(flag);

		if (auto voice = EngineInstance->impl->GetVoiceData(Handle); voice != nullptr)
			return (voice->Flags & voiceFlag);
		return false;
	}

	void Voice::SetInternalFlag(u16 flag, bool value)
	{
		static_assert(sizeof(flag) == sizeof(VoiceFlags));
		const auto voiceFlag = static_cast<VoiceFlags>(flag);

		if (auto voice = EngineInstance->impl->GetVoiceData(Handle); voice != nullptr)
		{
			if (value)
				voice->Flags |= voiceFlag;
			else
				voice->Flags &= ~voiceFlag;
		}
	}

	CallbackReceiver::CallbackReceiver(std::function<void(void)> callback) : OnAudioCallback(std::move(callback))
	{
		auto& impl = EngineInstance->impl;

		for (auto& receiver : impl->RegisteredCallbackReceivers)
		{
			if (receiver != nullptr)
				continue;

			receiver = this;
			return;
		}

		impl->RegisteredCallbackReceivers.push_back(this);
	}

	CallbackReceiver::~CallbackReceiver()
	{
		auto& impl = EngineInstance->impl;

		for (auto& receiver : impl->RegisteredCallbackReceivers)
		{
			if (receiver == this)
				receiver = nullptr;
		}
	}
}
