#include "AudioEngine.h"
#include "AudioInstance.h"
#include <assert.h>
#include <functional>
#include <malloc.h>
#include <RtAudio.h>

// defined in rtaudio/asio.cpp
typedef long ASIOError;
ASIOError ASIOControlPanel();

AudioEngine* AudioEngine::engineInstance = nullptr;

AudioEngine::AudioEngine()
{
}

AudioEngine::~AudioEngine()
{
}

void AudioEngine::Initialize()
{
	engineInstance = this;

	SetAudioApi(GetDefaultAudioApi());

	audioInstances.reserve(64);
	tempOutputBuffer = new int16_t[MAX_BUFFER_SIZE * GetChannelCount()];
}

void AudioEngine::Dispose()
{
	if (GetRtAudio() != nullptr)
		delete GetRtAudio();

	if (tempOutputBuffer != nullptr)
		delete[] tempOutputBuffer;
}

void AudioEngine::SetAudioApi(AudioApi audioApi)
{
	this->audioApi = audioApi;
	bool wasStreamRunning = GetIsStreamOpen() && GetIsStreamRunning();

	if (GetIsStreamRunning())
		StopStream();

	if (GetIsStreamOpen())
		CloseStream();

	if (GetRtAudio() != nullptr)
		delete GetRtAudio();

	rtAudio = new RtAudio(GetRtAudioApi(audioApi));

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
		fprintf(stderr, "AudioEngine::OpenAccess(): Failed: %s\n", exception.getMessage().c_str());
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

AudioCallbackResult AudioEngine::InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount, double streamTime)
{
	this->bufferSize = bufferFrameCount;

	lastCallbackStreamTime = callbackStreamTime;
	callbackStreamTime = streamTime;
	callbackLatency = callbackStreamTime - lastCallbackStreamTime;
	callbackRunning = true;

	for (auto& callbackReceiver : callbackReceivers)
		callbackReceiver->OnAudioCallback();

	// need to clear out the buffer from the previous call
	int64_t samplesInBuffer = bufferFrameCount * GetChannelCount();
	int64_t byteBufferSize = samplesInBuffer * sizeof(int16_t);
	memset(outputBuffer, 0, byteBufferSize);

	// 2 channels * 64 frames * sizeof(int16_t) -> 256 bytes inside the outputBuffer
	// 64 samples for each ear

	int audioInstancesSize = audioInstances.size();
	for (size_t i = 0; i < audioInstancesSize; i++)
	{
		AudioInstance* audioInstance = audioInstances[i].get();

		if (audioInstance->GetAppendRemove() || (audioInstance->IsSampleProviderValid() && audioInstance->GetHasReachedEnd() && audioInstance->GetOnFinishedAction() == AUDIO_FINISHED_REMOVE))
		{
			audioInstance->SetHasBeenRemoved(true);
			
			audioInstances.erase(audioInstances.begin() + i);
			audioInstancesSize--;
			
			continue;
		}

		// don't bother with these
		if (!audioInstance->IsSampleProviderValid() || !audioInstance->GetIsPlaying())
			continue;

		if (audioInstance->GetHasReachedEnd())
		{
			if (audioInstance->GetIsLooping())
				audioInstance->Restart();

			if (!audioInstance->GetPlayPastEnd())
				continue;
		}

		int64_t samplesRead = audioInstance->GetSampleProvider()->ReadSamples(tempOutputBuffer, audioInstance->GetSamplePosition(), samplesInBuffer);
		audioInstance->IncrementSamplePosition(samplesRead);

		if (!audioInstance->GetPlayPastEnd() && audioInstance->GetHasReachedEnd())
			audioInstance->SetSamplePosition(audioInstance->GetSampleCount());

		for (int64_t i = 0; i < samplesRead; i++)
			outputBuffer[i] = MixSamples(outputBuffer[i], tempOutputBuffer[i] * audioInstance->GetVolume());
	}

	for (int64_t i = 0; i < samplesInBuffer; i++)
		outputBuffer[i] *= GetMasterVolume();

	callbackRunning = false;
	return AUDIO_CALLBACK_CONTINUE;
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

void AudioEngine::AddAudioInstance(std::shared_ptr<AudioInstance> audioInstance)
{
	bool unique = true;
	for (const auto &instance : audioInstances)
	{
		if (instance == audioInstance)
		{
			unique = false;
			break;
		}
	}
	
	if (unique)
	{
		audioInstances.push_back(audioInstance);
	}
}

void AudioEngine::PlaySound(ISampleProvider* sampleProvider, float volume, const char* name)
{
	AddAudioInstance(std::make_shared<AudioInstance>(sampleProvider, true, AUDIO_FINISHED_REMOVE, volume, name));
}

void AudioEngine::ShowControlPanel()
{
	if (GetActiveAudioApi() == AUDIO_API_ASIO)
		ASIOControlPanel();
}

void AudioEngine::AddCallbackReceiver(ICallbackReceiver* callbackReceiver)
{
	callbackReceivers.push_back(callbackReceiver);
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
	return GetInstance()->InternalAudioCallback(static_cast<int16_t*>(outputBuffer), bufferFrames, streamTime);
}
