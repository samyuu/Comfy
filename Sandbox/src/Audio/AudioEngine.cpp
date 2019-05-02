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

AudioCallbackResult AudioEngine::InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount)
{
	this->bufferSize = bufferFrameCount;

	//memcpy(outputBuffer, currentSampleBuffer, sizeof(currentSampleBuffer));
	//printf("AudioEngine::InternalAudioCallback(): bufferFrameCount: %d\n", bufferFrameCount);

	bufferFrameCount;	// 64
	GetChannelCount();	// 2

	// 2 channels * 64 frames * sizeof(int16_t) -> 256 bytes inside the outputBuffer
	// 64 samples for each ear

	// need to clear out the buffer from the previous call
	memset(outputBuffer, 0, bufferFrameCount * GetChannelCount() * sizeof(int16_t));

	//for (size_t i = 0; i < bufferFrameCount * GetChannelCount(); i += 1)
	//	outputBuffer[i] = INT16_MAX * GetMasterVolume();

	for (size_t i = 0; i < bufferFrameCount * GetChannelCount(); i += 1)
	{
		double sine = sin(GetStreamTime() * 2000000.0f);
		short value = sine * INT16_MAX * GetMasterVolume();
		outputBuffer[i] = value;
	}

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
	assert(bufferSize < MAX_BUFFER_SIZE);

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

RtAudio::StreamParameters* AudioEngine::GetStreamOutputParameters()
{
	if (streamOutputParameter == nullptr)
		streamOutputParameter = new RtAudio::StreamParameters();

	streamOutputParameter->deviceId = GetRtAudio()->getDefaultOutputDevice();
	streamOutputParameter->nChannels = GetChannelCount();;
	streamOutputParameter->firstChannel = 0;

	return streamOutputParameter;
}

RtAudio::StreamParameters* AudioEngine::GetStreamInputParameters()
{
	return nullptr;
}

int AudioEngine::InternalStaticAudioCallback(void* outputBuffer, void*, uint32_t bufferFrames, double, RtAudioStreamStatus, void*)
{
	return GetInstance()->InternalAudioCallback(static_cast<int16_t*>(outputBuffer), bufferFrames);
}
