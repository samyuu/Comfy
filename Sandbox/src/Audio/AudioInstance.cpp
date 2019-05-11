#include "AudioInstance.h"
#include <assert.h>

AudioInstance::AudioInstance(ISampleProvider* sampleProvider) : sampleProvider(sampleProvider)
{
	//assert(sampleProvider);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, const char* name) : AudioInstance(sampleProvider)
{
	SetName(name);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, bool playing) : AudioInstance(sampleProvider)
{
	SetIsPlaying(playing);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, bool playing, const char* name) : AudioInstance(sampleProvider, playing)
{
	SetName(name);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction) : AudioInstance(sampleProvider, playing)
{
	SetOnFinishedAction(finishedAction);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume) : AudioInstance(sampleProvider, playing, finishedAction)
{
	SetVolume(volume);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume, const char* name) : AudioInstance(sampleProvider, playing, finishedAction, volume)
{
	SetName(name);
}

AudioInstance::~AudioInstance()
{

}

ISampleProvider* AudioInstance::GetSampleProvider() 
{ 
	return sampleProvider; 
}
void AudioInstance::SetSampleProvider(ISampleProvider* provider) 
{ 
	assert(provider != nullptr);
	sampleProvider = provider; 
}
bool AudioInstance::IsSampleProviderValid()
{
	return sampleProvider != nullptr;
}

TimeSpan AudioInstance::GetPosition()
{
	return SamplesToTimeSpan(GetSamplePosition());
}

void AudioInstance::SetPosition(TimeSpan value)
{
	SetSamplePosition(TimeSpanToSamples(value));
}

void AudioInstance::Restart()
{
	SetSamplePosition(0);
}

TimeSpan AudioInstance::GetDuration()
{
	return SamplesToTimeSpan(GetSampleCount());
}

const char* AudioInstance::GetName()
{
	return name;
}
void AudioInstance::SetName(const char* value)
{
	if (value != nullptr)
		this->name = value;
}

float AudioInstance::GetVolume()
{
	return volume;
}
void AudioInstance::SetVolume(float value)
{
	volume = std::clamp(value, MIN_VOLUME, MAX_VOLUME);
}

bool AudioInstance::GetIsPlaying()
{
	return isPlaying;
}
void AudioInstance::SetIsPlaying(bool value)
{
	isPlaying = value;
}

bool AudioInstance::GetIsLooping()
{
	return isLooping;
}
void AudioInstance::SetIsLooping(bool value)
{
	isLooping = value;
}

bool AudioInstance::GetAppendRemove()
{
	return appendRemove;
}
void AudioInstance::SetAppendRemove(bool value)
{
	appendRemove = value;
}

bool AudioInstance::GetHasBeenRemoved()
{
	return hasBeenRemoved;
}
bool AudioInstance::GetHasReachedEnd()
{
	return GetSamplePosition() >= GetSampleCount();
}

AudioFinishedAction AudioInstance::GetOnFinishedAction()
{
	return onFinishedAction;
}
void AudioInstance::SetOnFinishedAction(AudioFinishedAction value)
{
	onFinishedAction = value;
}

size_t AudioInstance::GetSamplePosition()
{
	return samplePosition;
}
void AudioInstance::SetSamplePosition(size_t value)
{
	samplePosition = std::clamp(value, (size_t)0, GetSampleCount());
}

size_t AudioInstance::GetSampleCount()
{
	return GetSampleProvider()->GetSampleCount();
}

uint32_t AudioInstance::GetSampleRate()
{
	return GetSampleProvider()->GetSampleRate();
}

uint32_t AudioInstance::GetChannelCount()
{
	return GetSampleProvider()->GetChannelCount();
}

TimeSpan AudioInstance::SamplesToTimeSpan(double samples)
{
	return SamplesToTimeSpan(samples, GetSampleRate(), GetChannelCount());
}

size_t AudioInstance::TimeSpanToSamples(TimeSpan time)
{
	return TimeSpanToSamples(time, GetSampleRate(), GetChannelCount());
}

inline TimeSpan AudioInstance::SamplesToTimeSpan(double samples, double sampleRate, double channelCount)
{
	return TimeSpan::FromSeconds((samples / channelCount) / sampleRate);
}

inline size_t AudioInstance::TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount)
{
	return (time.TotalSeconds() * sampleRate * channelCount);
}