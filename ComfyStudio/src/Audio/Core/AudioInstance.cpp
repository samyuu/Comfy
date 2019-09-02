#include "AudioInstance.h"
#include <assert.h>

namespace Audio
{
	AudioInstance::AudioInstance(RefPtr<ISampleProvider> sampleProvider, bool playing, const char* name) 
		: AudioInstance(sampleProvider, playing, AudioFinishedAction::None, AudioEngine::MaxVolume, name)
	{
	}
	
	AudioInstance::AudioInstance(RefPtr<ISampleProvider> sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume, const char* name) 
		: sampleProvider(sampleProvider), isPlaying(playing), onFinishedAction(finishedAction), volume(volume), name(name)
	{
	}

	AudioInstance::~AudioInstance()
	{

	}

	const RefPtr<ISampleProvider>& AudioInstance::GetSampleProvider() const
	{
		return sampleProvider;
	}

	void AudioInstance::SetSampleProvider(const RefPtr<ISampleProvider>& value)
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

	int64_t AudioInstance::GetFramePosition() const
	{
		return framePosition;
	}

	void AudioInstance::SetFramePosition(int64_t value)
	{
		framePosition = value;
	}

	int64_t AudioInstance::GetFrameCount() const
	{
		return sampleProvider->GetFrameCount();
	}

	uint32_t AudioInstance::GetSampleRate() const
	{
		return sampleProvider->GetSampleRate();
	}

	uint32_t AudioInstance::GetChannelCount() const
	{
		return sampleProvider->GetChannelCount();
	}

	TimeSpan AudioInstance::FramesToTimeSpan(double frames) const
	{
		return FramesToTimeSpan(frames, GetSampleRate());
	}

	int64_t AudioInstance::TimeSpanToFrames(TimeSpan time) const
	{
		return TimeSpanToFrames(time, GetSampleRate());
	}

	inline TimeSpan AudioInstance::FramesToTimeSpan(double frames, double sampleRate)
	{
		return TimeSpan::FromSeconds(frames / sampleRate);
	}

	inline int64_t AudioInstance::TimeSpanToFrames(TimeSpan time, double sampleRate)
	{
		return static_cast<int64_t>(time.TotalSeconds() * sampleRate);
	}
}