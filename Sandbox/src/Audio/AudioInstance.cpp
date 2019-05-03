#include "AudioInstance.h"
#include <assert.h>

AudioInstance::AudioInstance(ISampleProvider* sampleProvider) : sampleProvider(sampleProvider)
{
	assert(sampleProvider);
}

AudioInstance::AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction) : AudioInstance(sampleProvider)
{
	SetIsPlaying(playing);
	SetOnFinishedAction(finishedAction);
}

AudioInstance::~AudioInstance()
{

}

TimeSpan AudioInstance::GetPosition()
{
	return SamplesToTimeSpan(GetSamplePosition());
}

void AudioInstance::SetPosition(TimeSpan value)
{
	SetSamplePosition(TimeSpanToSamples(value));
}

TimeSpan AudioInstance::GetDuration()
{
	return SamplesToTimeSpan(GetSampleCount());
}

inline TimeSpan AudioInstance::SamplesToTimeSpan(double samples)
{
	return SamplesToTimeSpan(samples, GetSampleRate(), GetChannelCount());
};

inline size_t AudioInstance::TimeSpanToSamples(TimeSpan time)
{
	return TimeSpanToSamples(time, GetSampleRate(), GetChannelCount());
};

inline TimeSpan AudioInstance::SamplesToTimeSpan(double samples, double sampleRate, double channelCount)
{
	return TimeSpan::FromSeconds((samples / channelCount) / sampleRate);
};

inline size_t AudioInstance::TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount)
{
	return (time.Seconds() * sampleRate * channelCount);
};