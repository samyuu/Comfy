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
	tempOutputBuffer = MakeUnique<int16_t[]>(MAX_BUFFER_SIZE * GetChannelCount());
}

void AudioEngine::Dispose()
{
	if (GetIsStreamOpen())
		StopStream();
}

void AudioEngine::SetAudioApi(AudioApi audioApi)
{
	this->audioApi = audioApi;
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
	masterVolume = std::clamp(value, MIN_VOLUME, MAX_VOLUME); 
}

bool AudioEngine::GetIsExclusiveMode() 
{ 
	return GetActiveAudioApi() == AudioApi::ASIO; 
}

void AudioEngine::CreateInstance()
{
	engineInstance = new AudioEngine();
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
	delete GetInstance();
	engineInstance = nullptr;
}

int16_t AudioEngine::MixSamples(int16_t sampleA, int16_t sampleB)
{
	const int32_t result = static_cast<int32_t>(sampleA) + static_cast<int32_t>(sampleB);
	typedef std::numeric_limits<short int> Range;

	if (Range::max() < result)
		return Range::max();
	else if (Range::min() > result)
		return Range::min();
	else
		return result;
}

RtAudio::Api AudioEngine::GetRtAudioApi(AudioApi audioApi)
{
	return (audioApi > AudioApi::Invalid && audioApi < AudioApi::Count) ? audioApis.at(static_cast<int>(audioApi)) : RtAudio::UNSPECIFIED;
}

AudioCallbackResult AudioEngine::InternalAudioCallback(int16_t* outputBuffer, uint32_t bufferFrameCount, double streamTime)
{
	this->bufferSize = bufferFrameCount;

	lastCallbackStreamTime = callbackStreamTime;
	callbackStreamTime = streamTime;
	callbackLatency = callbackStreamTime - lastCallbackStreamTime;
	callbackRunning = true;

	for (auto& callbackReceiver : callbackReceivers)
	{
		if (callbackReceiver != nullptr)
			callbackReceiver->OnAudioCallback();
	}

	// need to clear out the buffer from the previous call
	int64_t samplesInBuffer = bufferFrameCount * GetChannelCount();
	int64_t byteBufferSize = samplesInBuffer * sizeof(int16_t);
	memset(outputBuffer, 0, byteBufferSize);

	// 2 channels * 64 frames * sizeof(int16_t) -> 256 bytes inside the outputBuffer
	// 64 samples for each ear

	size_t audioInstancesSize = audioInstances.size();
	for (size_t i = 0; i < audioInstancesSize; i++)
	{
		AudioInstance* audioInstance = audioInstances[i].get();

		if (audioInstance->GetAppendRemove() || (audioInstance->IsSampleProviderValid() && audioInstance->GetHasReachedEnd() && audioInstance->GetOnFinishedAction() == AudioFinishedAction::Remove))
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

		int64_t samplesRead = audioInstance->GetSampleProvider()->ReadSamples(tempOutputBuffer.get(), audioInstance->GetSamplePosition(), samplesInBuffer);
		audioInstance->IncrementSamplePosition(samplesRead);

		if (!audioInstance->GetPlayPastEnd() && audioInstance->GetHasReachedEnd())
			audioInstance->SetSamplePosition(audioInstance->GetSampleCount());

		for (int64_t i = 0; i < samplesRead; i++)
			outputBuffer[i] = MixSamples(outputBuffer[i], static_cast<int16_t>(tempOutputBuffer[i] * audioInstance->GetVolume()));
	}

	for (int64_t i = 0; i < samplesInBuffer; i++)
		outputBuffer[i] = static_cast<int16_t>(outputBuffer[i] * GetMasterVolume());

	callbackRunning = false;
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

void AudioEngine::AddAudioInstance(RefPtr<AudioInstance> audioInstance)
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
	AddAudioInstance(MakeRefPtr<AudioInstance>(sampleProvider, true, AudioFinishedAction::Remove, volume, name));
}

void AudioEngine::ShowControlPanel()
{
	if (GetActiveAudioApi() == AudioApi::ASIO)
		ASIOControlPanel();
}

void AudioEngine::AddCallbackReceiver(ICallbackReceiver* callbackReceiver)
{
	callbackReceivers.push_back(callbackReceiver);
}

void AudioEngine::RemoveCallbackReceiver(ICallbackReceiver* callbackReceiver)
{
	for (size_t i = 0; i < callbackReceivers.size(); i++)
	{
		if (callbackReceivers[i] == callbackReceiver)
			callbackReceivers[i] = nullptr;
	}
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
