#pragma once
#include "ISampleProvider.h"
#include "AudioEngine.h"
#include "../TimeSpan.h"

enum AudioFinishedAction : byte
{
	AUDIO_FINISHED_NONE,
	AUDIO_FINISHED_REMOVE,
	AUDIO_FINISHED_DELETE,
	AUDIO_FINISHED_COUNT,
};

class AudioInstance
{
public:
	AudioInstance(ISampleProvider* sampleProvider);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction);
	~AudioInstance();

	inline ISampleProvider* GetSampleProvider() { return sampleProvider; };

	inline float GetVolume() { return volume; };
	inline void SetVolume(float value) { volume = std::clamp(value, MIN_VOLUME, MAX_VOLUME); };

	inline bool GetIsPlaying() { return isPlaying; };
	inline void SetIsPlaying(bool value) { isPlaying = value; };

	inline bool GetHasBeenRemoved() { return hasBeenRemoved; };
	inline void SetHasBeenRemoved(bool value) { hasBeenRemoved = value; };

	inline AudioFinishedAction GetOnFinishedAction() { return onFinishedAction; };
	inline void SetOnFinishedAction(AudioFinishedAction value) { onFinishedAction = value; };

	inline bool GetHasReachedEnd() { return GetSamplePosition() >= GetSampleCount(); };

	inline size_t GetSamplePosition() { return samplePosition; };
	inline void SetSamplePosition(size_t value) { samplePosition = std::clamp(value, (size_t)0, GetSampleCount()); };
	inline void IncrementSamplePosition(size_t value) { SetSamplePosition(GetSamplePosition() + value); };

	inline size_t GetSampleCount() { return GetSampleProvider()->GetSampleCount(); };
	inline uint32_t GetSampleRate() { return GetSampleProvider()->GetSampleRate(); };
	inline uint32_t GetChannelCount() { return GetSampleProvider()->GetChannelCount(); };

	TimeSpan GetPosition();
	void SetPosition(TimeSpan value);

	TimeSpan GetDuration();

private:
	float volume = MAX_VOLUME;
	bool isPlaying = false;
	bool hasBeenRemoved = false;
	AudioFinishedAction onFinishedAction = AUDIO_FINISHED_NONE;

	// playback position measured as a sample index
	size_t samplePosition = 0;

	ISampleProvider* sampleProvider = nullptr;

	TimeSpan SamplesToTimeSpan(double samples);
	size_t TimeSpanToSamples(TimeSpan time);

	static TimeSpan SamplesToTimeSpan(double samples, double sampleRate, double channelCount);
	static size_t TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount);
};