#include "AudioEngine.h"
#include <assert.h>
#include <functional>

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
}

void AudioEngine::Dispose()
{
	if (GetRtAudio() != nullptr)
		delete GetRtAudio();
}

void AudioEngine::SetAudioApi(AudioApi audioApi)
{
	if (GetActiveAudioApi() == audioApi)
		return;
	
	this->audioApi = audioApi;
	
	if (GetIsStreamRunning())
		StopStream();

	if (GetIsStreamOpen())
		CloseAccess();

	if (GetRtAudio() != nullptr)
		delete GetRtAudio();
	
	rtAudio = new RtAudio(GetRtAudioApi(audioApi));
}

void AudioEngine::OpenAccess()
{
	auto outputParameters = GetStreamOutputParameters();
	auto inputParameters = GetStreamInputParameters();
	auto format = GetStreamFormat();
	auto sampleRate = GetSampleRate();
	auto bufferSize = GetBufferSize();
	void* extraData = nullptr;

	try
	{
		GetRtAudio()->openStream(&outputParameters, inputParameters, format, sampleRate, &bufferSize, &InternalStaticAudioCallback, extraData);
		isStreamOpen = true;
	}
	catch (const RtAudioError& exception)
	{
		fprintf(stderr, "AudioEngine::OpenAccess(): Failed: %s\n", exception.getMessage().c_str());
		return;
	}
}

void AudioEngine::CloseAccess()
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

AudioCallbackResult AudioEngine::InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount)
{
	//memcpy(outputBuffer, currentSampleBuffer, sizeof(currentSampleBuffer));
	printf("AudioEngine::InternalAudioCallback(): yoo\n");

	bufferFrameCount;	// 64
	GetChannelCount();	// 2

	// 2 channels * 64 frames * sizeof(int16_t) -> 256 bytes inside the outputBuffer
	// 64 samples for each ear

	// need to clear out the buffer from the previous call
	memset(outputBuffer, 0, bufferFrameCount * GetChannelCount() * sizeof(int16_t));

	for (size_t i = 0; i < bufferFrameCount; i+=2)
		outputBuffer[i] = SHRT_MAX * GetMasterVolume();

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

RtAudio::StreamParameters AudioEngine::GetStreamOutputParameters()
{
	RtAudio::StreamParameters streamParameters;
	streamParameters.deviceId = GetRtAudio()->getDefaultOutputDevice();
	streamParameters.nChannels = GetChannelCount();;
	streamParameters.firstChannel = 0;

	return streamParameters;
}

RtAudio::StreamParameters* AudioEngine::GetStreamInputParameters()
{
	return nullptr;
}

int AudioEngine::InternalStaticAudioCallback(void* outputBuffer, void*, uint32_t bufferFrames, double, RtAudioStreamStatus, void*)
{
	return GetInstance()->InternalAudioCallback(static_cast<int16_t*>(outputBuffer), bufferFrames);
}
