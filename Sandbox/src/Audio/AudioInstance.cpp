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

ISampleProvider* AudioInstance::GetSampleProvider() const
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

TimeSpan AudioInstance::GetPosition() const
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

TimeSpan AudioInstance::GetDuration() const
{
	return SamplesToTimeSpan(GetSampleCount());
}

const char* AudioInstance::GetName() const
{
	return name;
}
void AudioInstance::SetName(const char* value)
{
	if (value != nullptr)
		this->name = value;
}

float AudioInstance::GetVolume() const
{
	return volume;
}
void AudioInstance::SetVolume(float value)
{
	volume = std::clamp(value, MIN_VOLUME, MAX_VOLUME);
}

bool AudioInstance::GetIsPlaying() const
{
	return isPlaying;
}
void AudioInstance::SetIsPlaying(bool value)
{
	isPlaying = value;
}

bool AudioInstance::GetIsLooping() const
{
	return isLooping;
}
void AudioInstance::SetIsLooping(bool value)
{
	isLooping = value;
}

bool AudioInstance::GetPlayPastEnd() const
{
	return playPastEnd;
}
void AudioInstance::SetPlayPastEnd(bool value)
{
	playPastEnd = value;
}

bool AudioInstance::GetAppendRemove() const
{
	return appendRemove;
}
void AudioInstance::SetAppendRemove(bool value)
{
	appendRemove = value;
}

bool AudioInstance::GetHasBeenRemoved() const
{
	return hasBeenRemoved;
}
bool AudioInstance::GetHasReachedEnd() const
{
	return GetSamplePosition() >= GetSampleCount();
}

AudioFinishedAction AudioInstance::GetOnFinishedAction() const
{
	return onFinishedAction;
}
void AudioInstance::SetOnFinishedAction(AudioFinishedAction value)
{
	onFinishedAction = value;
}

int64_t AudioInstance::GetSamplePosition() const
{
	return samplePosition;
}
void AudioInstance::SetSamplePosition(int64_t value)
{
	samplePosition = value; 
}

int64_t AudioInstance::GetSampleCount() const
{
	return GetSampleProvider()->GetSampleCount();
}

uint32_t AudioInstance::GetSampleRate() const
{
	return GetSampleProvider()->GetSampleRate();
}

uint32_t AudioInstance::GetChannelCount() const
{
	return GetSampleProvider()->GetChannelCount();
}

TimeSpan AudioInstance::SamplesToTimeSpan(double samples) const
{
	return SamplesToTimeSpan(samples, GetSampleRate(), GetChannelCount());
}

int64_t AudioInstance::TimeSpanToSamples(TimeSpan time) const
{
	return TimeSpanToSamples(time, GetSampleRate(), GetChannelCount());
}

inline TimeSpan AudioInstance::SamplesToTimeSpan(double samples, double sampleRate, double channelCount)
{
	return TimeSpan::FromSeconds((samples / channelCount) / sampleRate);
}

inline int64_t AudioInstance::TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount)
{
	return (time.TotalSeconds() * sampleRate * channelCount);
}