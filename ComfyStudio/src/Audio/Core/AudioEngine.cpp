#include "AudioEngine.h"
#include "AudioInstance.h"
#include "Audio/Decoder/AudioDecoderFactory.h"
#include "Core/Logger.h"
#include <functional>
#include <RtAudio.h>
#include <assert.h>

namespace Audio
{
	UniquePtr<AudioEngine> AudioEngine::engineInstance = nullptr;

	AudioEngine::AudioEngine()
	{
	}

	AudioEngine::~AudioEngine()
	{
	}

	void AudioEngine::Initialize()
	{
		SetAudioApi(GetDefaultAudioApi());

		channelMixer.SetTargetChannels(GetChannelCount());
		channelMixer.SetMixingBehavior(ChannelMixer::MixingBehavior::Mix);

		constexpr size_t initialCapacity = 64;
		audioInstances.reserve(initialCapacity);
		tempOutputBuffer.resize(MAX_BUFFER_SIZE * GetChannelCount());
	}

	void AudioEngine::Dispose()
	{
		if (GetIsStreamOpen())
			StopStream();
	}

	void AudioEngine::SetAudioApi(AudioApi value)
	{
		audioApi = value;
		bool wasStreamRunning = GetIsStreamOpen() && GetIsStreamRunning();

		if (GetIsStreamRunning())
			StopStream();

		if (GetIsStreamOpen())
			CloseStream();

		rtAudio = MakeUnique<RtAudio>(GetRtAudioApi(audioApi));

		if (wasStreamRunning)
		{
			OpenStream();
			StartStream();
		}
	}

	void AudioEngine::OpenStream()
	{
		if (GetIsStreamOpen())
			return;

		auto outputParameters = GetStreamOutputParameters();
		auto inputParameters = GetStreamInputParameters();
		auto format = GetStreamFormat();
		auto sampleRate = GetSampleRate();
		auto bufferSize = GetBufferSize();
		void* extraData = nullptr;

		try
		{
			GetRtAudio()->openStream(outputParameters, inputParameters, format, sampleRate, &bufferSize, &InternalStaticAudioCallback, extraData);
			isStreamOpen = true;
		}
		catch (const RtAudioError& exception)
		{
			Logger::LogErrorLine(__FUNCTION__"(): Failed: %s", exception.getMessage().c_str());
			return;
		}
	}

	void AudioEngine::CloseStream()
	{
		if (!isStreamOpen)
			return;

		isStreamOpen = isStreamRunning = false;
		GetRtAudio()->closeStream();
	}

	void AudioEngine::StartStream()
	{
		if (!isStreamOpen || isStreamRunning)
			return;

		isStreamRunning = true;
		GetRtAudio()->startStream();
	}

	void AudioEngine::StopStream()
	{
		if (!isStreamOpen || !isStreamRunning)
			return;

		isStreamRunning = false;
		GetRtAudio()->stopStream();
	}

	double AudioEngine::GetStreamTime()
	{
		return GetRtAudio() && GetIsStreamOpen() ? GetRtAudio()->getStreamTime() : 0.0;
	};

	void AudioEngine::SetStreamTime(double value)
	{
		if (GetRtAudio() != nullptr)
		{
			GetRtAudio()->setStreamTime(value);
		}
	};

	double AudioEngine::GetCallbackLatency()
	{
		return callbackLatency;
	};

	float AudioEngine::GetMasterVolume()
	{
		return masterVolume;
	}

	void AudioEngine::SetMasterVolume(float value)
	{
		masterVolume = std::clamp(value, MinVolume, MaxVolume);
	}

	bool AudioEngine::GetIsExclusiveMode()
	{
		return GetActiveAudioApi() == AudioApi::ASIO;
	}

	void AudioEngine::CreateInstance()
	{
		engineInstance.reset(new AudioEngine());
	}

	void AudioEngine::InitializeInstance()
	{
		GetInstance()->Initialize();
	}

	void AudioEngine::DisposeInstance()
	{
		GetInstance()->Dispose();
	}

	void AudioEngine::DeleteInstance()
	{
		engineInstance = nullptr;
	}

	RtAudio::Api AudioEngine::GetRtAudioApi(AudioApi audioApi)
	{
		return (audioApi > AudioApi::Invalid && audioApi < AudioApi::Count) ? audioApis.at(static_cast<int>(audioApi)) : RtAudio::UNSPECIFIED;
	}

	AudioEngine::AudioCallbackResult AudioEngine::InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount, double streamTime)
	{
		bufferSize = bufferFrameCount;
		callbackStreamTime = streamTime;

		lastCallbackStreamTime = callbackStreamTime;
		callbackLatency = callbackStreamTime - lastCallbackStreamTime;

		for (auto& callbackReceiver : callbackReceivers)
		{
			if (callbackReceiver != nullptr)
				callbackReceiver->OnAudioCallback();
		}

		// need to clear out the buffer from the previous call
		uint32_t targetChannels = GetChannelCount();
		std::fill(outputBuffer, outputBuffer + bufferFrameCount * targetChannels, 0);

		audioInstancesMutex.lock();
		{
			// 2 channels * 64 frames * sizeof(int16_t) -> 256 bytes inside the outputBuffer
			// 64 samples for each ear

			size_t audioInstancesSize = audioInstances.size();
			for (size_t i = 0; i < audioInstancesSize; i++)
			{
				const RefPtr<AudioInstance>& audioInstance = audioInstances[i];

				if (audioInstance->GetAppendRemove() || (audioInstance->HasSampleProvider() && audioInstance->GetHasReachedEnd() && audioInstance->GetOnFinishedAction() == AudioFinishedAction::Remove))
				{
					audioInstance->SetHasBeenRemoved(true);

					audioInstances.erase(audioInstances.begin() + i);
					audioInstancesSize--;

					continue;
				}

				// don't bother with these
				if (!audioInstance->HasSampleProvider() || !audioInstance->GetIsPlaying())
					continue;

				if (audioInstance->GetHasReachedEnd())
				{
					if (audioInstance->GetIsLooping())
						audioInstance->Restart();

					if (!audioInstance->GetPlayPastEnd())
						continue;
				}

				int64_t framesRead = 0;
				uint32_t sourceChannels = audioInstance->GetChannelCount();

				if (sourceChannels != targetChannels)
				{
					channelMixer.SetSourceChannels(sourceChannels);

					framesRead = channelMixer.MixChannels(audioInstance->GetSampleProvider().get(), tempOutputBuffer.data(), audioInstance->GetFramePosition(), bufferFrameCount);
				}
				else
				{
					framesRead = audioInstance->GetSampleProvider()->ReadSamples(tempOutputBuffer.data(), audioInstance->GetFramePosition(), bufferFrameCount, targetChannels);
				}

				audioInstance->IncrementFramePosition(framesRead);

				if (!audioInstance->GetPlayPastEnd() && audioInstance->GetHasReachedEnd())
					audioInstance->SetFramePosition(audioInstance->GetFrameCount());

				for (int64_t i = 0; i < framesRead * targetChannels; i++)
					outputBuffer[i] = SampleMixer::MixSamples(outputBuffer[i], static_cast<int16_t>(tempOutputBuffer[i] * audioInstance->GetVolume()));
			}
		}
		audioInstancesMutex.unlock();

		for (int64_t i = 0; i < bufferFrameCount * targetChannels; i++)
			outputBuffer[i] = static_cast<int16_t>(outputBuffer[i] * GetMasterVolume());

		return AudioCallbackResult::Continue;
	}

	size_t AudioEngine::GetDeviceCount()
	{
		return GetRtAudio() == nullptr ? -1 : GetRtAudio()->getDeviceCount();
	}

	RtAudio::DeviceInfo AudioEngine::GetDeviceInfo(uint32_t device)
	{
		return GetRtAudio()->getDeviceInfo(device);
	}

	void AudioEngine::SetBufferSize(uint32_t bufferSize)
	{
		assert(bufferSize <= MAX_BUFFER_SIZE);

		if (this->bufferSize == bufferSize)
			return;

		this->bufferSize = bufferSize;

		bool wasStreamOpen = GetIsStreamOpen();
		bool wasStreamRunning = GetIsStreamRunning();

		if (wasStreamOpen)
		{
			CloseStream();
			OpenStream();
		}

		if (wasStreamRunning && GetIsStreamOpen())
			StartStream();
	}

	void AudioEngine::AddAudioInstance(const RefPtr<AudioInstance>& audioInstance)
	{
		audioInstancesMutex.lock();
		{
			audioInstances.push_back(audioInstance);
		}
		audioInstancesMutex.unlock();
	}

	void AudioEngine::PlaySound(const RefPtr<ISampleProvider>& sampleProvider, float volume, const char* name)
	{
		AddAudioInstance(MakeRef<AudioInstance>(sampleProvider, true, AudioFinishedAction::Remove, volume, name));
	}

	void AudioEngine::ShowControlPanel()
	{
		if (GetActiveAudioApi() != AudioApi::ASIO)
			return;

		// defined in rtaudio/asio.cpp
		long ASIOControlPanel();

		ASIOControlPanel();
	}

	void AudioEngine::AddCallbackReceiver(ICallbackReceiver* callbackReceiver)
	{
		callbackReceivers.push_back(callbackReceiver);
	}

	void AudioEngine::RemoveCallbackReceiver(ICallbackReceiver* callbackReceiver)
	{
		for (auto& receiver : callbackReceivers)
		{
			if (receiver == callbackReceiver)
				receiver = nullptr;
		}
	}

	RefPtr<MemorySampleProvider> AudioEngine::LoadAudioFile(const String& filePath)
	{
		return AudioDecoderFactory::GetInstance()->DecodeFile(filePath);
	}

	uint32_t AudioEngine::GetDeviceId()
	{
		// TODO: store user preference
		return GetRtAudio()->getDefaultOutputDevice();
	}

	StreamParameters* AudioEngine::GetStreamOutputParameters()
	{
		streamOutputParameter.deviceId = GetDeviceId();
		streamOutputParameter.nChannels = GetChannelCount();;
		streamOutputParameter.firstChannel = 0;

		return &streamOutputParameter;
	}

	StreamParameters* AudioEngine::GetStreamInputParameters()
	{
		return nullptr;
	}

	int AudioEngine::InternalStaticAudioCallback(void* outputBuffer, void*, uint32_t bufferFrames, double streamTime, RtAudioStreamStatus, void*)
	{
		return static_cast<int>(GetInstance()->InternalAudioCallback(static_cast<int16_t*>(outputBuffer), bufferFrames, streamTime));
	}
}