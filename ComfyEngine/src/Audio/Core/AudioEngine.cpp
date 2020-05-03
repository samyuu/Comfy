#include "AudioEngine.h"
#include "AudioInstance.h"
#include "Audio/Decoder/AudioDecoderFactory.h"
#include "Core/Logger.h"
#include <RtAudio.h>
#include <functional>
#include <assert.h>

namespace Comfy::Audio
{
	std::unique_ptr<AudioEngine> AudioEngine::engineInstance = nullptr;

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

		rtAudio = std::make_unique<RtAudio>(GetRtAudioApi(audioApi));

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

#if 0
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
#else
		GetRtAudio()->openStream(outputParameters, inputParameters, format, sampleRate, &bufferSize, &InternalStaticAudioCallback, extraData);
		isStreamOpen = true;
#endif
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
	}

	void AudioEngine::SetStreamTime(double value)
	{
		if (GetRtAudio() != nullptr)
		{
			GetRtAudio()->setStreamTime(value);
		}
	}

	double AudioEngine::GetCallbackLatency()
	{
		return callbackLatency;
	}

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

	AudioEngine::AudioCallbackResult AudioEngine::InternalAudioCallback(i16* outputBuffer, u32 bufferFrameCount, double streamTime)
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
		u32 targetChannels = GetChannelCount();
		std::fill(outputBuffer, outputBuffer + bufferFrameCount * targetChannels, 0);

		audioInstancesMutex.lock();
		{
			// 2 channels * 64 frames * sizeof(i16) -> 256 bytes inside the outputBuffer
			// 64 samples for each ear

			size_t audioInstancesSize = audioInstances.size();
			for (size_t i = 0; i < audioInstancesSize; i++)
			{
				const std::shared_ptr<AudioInstance>& audioInstance = audioInstances[i];

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

				i64 framesRead = 0;
				u32 sourceChannels = audioInstance->GetChannelCount();

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

				for (i64 i = 0; i < framesRead * targetChannels; i++)
					outputBuffer[i] = SampleMixer::MixSamples(outputBuffer[i], static_cast<i16>(tempOutputBuffer[i] * audioInstance->GetVolume()));
			}
		}
		audioInstancesMutex.unlock();

		for (i64 i = 0; i < bufferFrameCount * targetChannels; i++)
			outputBuffer[i] = static_cast<i16>(outputBuffer[i] * GetMasterVolume());

		return AudioCallbackResult::Continue;
	}

	size_t AudioEngine::GetDeviceCount()
	{
		return GetRtAudio() == nullptr ? -1 : GetRtAudio()->getDeviceCount();
	}

	RtAudio::DeviceInfo AudioEngine::GetDeviceInfo(u32 device)
	{
		return GetRtAudio()->getDeviceInfo(device);
	}

	void AudioEngine::SetBufferSize(u32 bufferSize)
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

	void AudioEngine::AddAudioInstance(const std::shared_ptr<AudioInstance>& audioInstance)
	{
		audioInstancesMutex.lock();
		{
			audioInstances.push_back(audioInstance);
		}
		audioInstancesMutex.unlock();
	}

	void AudioEngine::PlaySound(const std::shared_ptr<ISampleProvider>& sampleProvider, float volume, const char* name)
	{
		AddAudioInstance(std::make_shared<AudioInstance>(sampleProvider, true, AudioFinishedAction::Remove, volume, name));
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

	std::shared_ptr<MemorySampleProvider> AudioEngine::LoadAudioFile(std::string_view filePath)
	{
		return AudioDecoderFactory::GetInstance()->DecodeFile(filePath);
	}

	u32 AudioEngine::GetDeviceID()
	{
		// TODO: store user preference
		return GetRtAudio()->getDefaultOutputDevice();
	}

	StreamParameters* AudioEngine::GetStreamOutputParameters()
	{
		streamOutputParameter.deviceId = GetDeviceID();
		streamOutputParameter.nChannels = GetChannelCount();
		streamOutputParameter.firstChannel = 0;

		return &streamOutputParameter;
	}

	StreamParameters* AudioEngine::GetStreamInputParameters()
	{
		return nullptr;
	}

	int AudioEngine::InternalStaticAudioCallback(void* outputBuffer, void*, u32 bufferFrames, double streamTime, RtAudioStreamStatus, void*)
	{
		return static_cast<int>(GetInstance()->InternalAudioCallback(static_cast<i16*>(outputBuffer), bufferFrames, streamTime));
	}
}
