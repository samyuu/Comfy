#include "AudioEngine.h"
#include "AudioInstance.h"
#include <assert.h>
#include <functional>
#include <malloc.h>

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

	if (streamOutputParameter != nullptr)
		delete streamOutputParameter;
}

void AudioEngine::SetAudioApi(AudioApi audioApi)
{
	if (GetActiveAudioApi() == audioApi)
		return;

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

	// need to clear out the buffer from the previous call
	size_t samplesInBuffer = bufferFrameCount * GetChannelCount();
	size_t byteBufferSize = samplesInBuffer * sizeof(int16_t);
	memset(outputBuffer, 0, byteBufferSize);

	// 2 channels * 64 frames * sizeof(int16_t) -> 256 bytes inside the outputBuffer
	// 64 samples for each ear

	int audioInstancesSize = audioInstances.size();
	for (size_t i = 0; i < audioInstancesSize; i++)
	{
		AudioInstance* audioInstance = audioInstances[i];

		if (audioInstance->GetAppendDelete() || (audioInstance->GetHasReachedEnd() && audioInstance->GetOnFinishedAction() != AUDIO_FINISHED_NONE))
		{
			audioInstance->SetHasBeenRemoved(true);
			
			audioInstances.erase(audioInstances.begin() + i);
			audioInstancesSize--;
			
			if (audioInstance->GetAppendDelete() || audioInstance->GetOnFinishedAction() == AUDIO_FINISHED_DELETE)
				delete audioInstance;
			
			continue;
		}

		if (!audioInstance->GetIsPlaying() || audioInstance->GetHasReachedEnd())
		{
			if (audioInstance->GetIsLooping())
				audioInstance->SetSamplePosition(0);
			continue;
		}

		size_t samplesRead = audioInstance->GetSampleProvider()->ReadSamples(tempOutputBuffer, audioInstance->GetSamplePosition(), samplesInBuffer);
		audioInstance->IncrementSamplePosition(samplesRead);

		for (size_t i = 0; i < samplesRead; i++)
			outputBuffer[i] = MixSamples(outputBuffer[i], tempOutputBuffer[i] * audioInstance->GetVolume());
	}

	for (size_t i = 0; i < samplesInBuffer; i++)
		outputBuffer[i] *= GetMasterVolume();

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

void AudioEngine::AddAudioInstance(AudioInstance* audioInstance)
{
	bool unique = true;
	for (const AudioInstance* instance : audioInstances)
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

void AudioEngine::PlaySound(ISampleProvider* sampleProvider, float volume)
{
	AddAudioInstance(new AudioInstance(sampleProvider, true, AUDIO_FINISHED_DELETE, volume));
}

StreamParameters* AudioEngine::GetStreamOutputParameters()
{
	if (streamOutputParameter == nullptr)
		streamOutputParameter = new StreamParameters();

	streamOutputParameter->deviceId = GetRtAudio()->getDefaultOutputDevice();
	streamOutputParameter->nChannels = GetChannelCount();;
	streamOutputParameter->firstChannel = 0;

	return streamOutputParameter;
}

StreamParameters* AudioEngine::GetStreamInputParameters()
{
	return nullptr;
}

int AudioEngine::InternalStaticAudioCallback(void* outputBuffer, void*, uint32_t bufferFrames, double streamTime, RtAudioStreamStatus, void*)
{
	return GetInstance()->InternalAudioCallback(static_cast<int16_t*>(outputBuffer), bufferFrames, streamTime);
}
