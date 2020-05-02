#include "AudioInstance.h"
#include <assert.h>

namespace Comfy::Audio
{
	AudioInstance::AudioInstance(std::shared_ptr<ISampleProvider> sampleProvider, bool playing, const char* name) 
		: AudioInstance(sampleProvider, playing, AudioFinishedAction::None, AudioEngine::MaxVolume, name)
	{
	}
	
	AudioInstance::AudioInstance(std::shared_ptr<ISampleProvider> sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume, const char* name) 
		: sampleProvider(sampleProvider), isPlaying(playing), onFinishedAction(finishedAction), volume(volume), name(name)
	{
	}

	AudioInstance::~AudioInstance()
	{

	}

	const std::shared_ptr<ISampleProvider>& AudioInstance::GetSampleProvider() const
	{
		return sampleProvider;
	}

	void AudioInstance::SetSampleProvider(const std::shared_ptr<ISampleProvider>& value)
	{
		assert(value != nullptr);
		sampleProvider = value;
	}

	bool AudioInstance::HasSampleProvider()
	{
		return sampleProvider != nullptr;
	}

	TimeSpan AudioInstance::GetPosition() const
	{
		return FramesToTimeSpan(static_cast<double>(GetFramePosition()));
	}

	void AudioInstance::SetPosition(TimeSpan value)
	{
		SetFramePosition(TimeSpanToFrames(value));
	}

	void AudioInstance::Restart()
	{
		SetFramePosition(0);
	}

	TimeSpan AudioInstance::GetDuration() const
	{
		return FramesToTimeSpan(static_cast<double>(GetFrameCount()));
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
		volume = std::clamp(value, AudioEngine::MinVolume, AudioEngine::MaxVolume);
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
		return GetFramePosition() >= GetFrameCount();
	}

	AudioFinishedAction AudioInstance::GetOnFinishedAction() const
	{
		return onFinishedAction;
	}

	void AudioInstance::SetOnFinishedAction(AudioFinishedAction value)
	{
		onFinishedAction = value;
	}

	i64 AudioInstance::GetFramePosition() const
	{
		return framePosition;
	}

	void AudioInstance::SetFramePosition(i64 value)
	{
		framePosition = value;
	}

	i64 AudioInstance::GetFrameCount() const
	{
		return sampleProvider->GetFrameCount();
	}

	u32 AudioInstance::GetSampleRate() const
	{
		return sampleProvider->GetSampleRate();
	}

	u32 AudioInstance::GetChannelCount() const
	{
		return sampleProvider->GetChannelCount();
	}

	TimeSpan AudioInstance::FramesToTimeSpan(double frames) const
	{
		return FramesToTimeSpan(frames, GetSampleRate());
	}

	i64 AudioInstance::TimeSpanToFrames(TimeSpan time) const
	{
		return TimeSpanToFrames(time, GetSampleRate());
	}

	TimeSpan AudioInstance::FramesToTimeSpan(double frames, double sampleRate)
	{
		return TimeSpan::FromSeconds(frames / sampleRate);
	}

	i64 AudioInstance::TimeSpanToFrames(TimeSpan time, double sampleRate)
	{
		return static_cast<i64>(time.TotalSeconds() * sampleRate);
	}
}
