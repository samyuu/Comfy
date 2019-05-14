#pragma once
#include "ISampleProvider.h"
#include "AudioEngine.h"
#include "../TimeSpan.h"
#include <string>

enum AudioFinishedAction : byte
{
	AUDIO_FINISHED_NONE,
	AUDIO_FINISHED_REMOVE,
	AUDIO_FINISHED_MAX,
};

class AudioInstance
{
public:
	friend class AudioEngine;
	
	// Constructors
	// ------------
	AudioInstance(ISampleProvider* sampleProvider);
	AudioInstance(ISampleProvider* sampleProvider, const char* name);
	AudioInstance(ISampleProvider* sampleProvider, bool playing);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, const char* name);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume, const char* name);

	// Destructors
	// -----------
	~AudioInstance();

	// Sample Provider
	ISampleProvider* GetSampleProvider();
	void SetSampleProvider(ISampleProvider* provider);
	bool IsSampleProviderValid();

	// Position
	TimeSpan GetPosition();
	void SetPosition(TimeSpan value);
	void Restart();

	// Duration
	TimeSpan GetDuration();

	// Name
	const char* GetName();
	void SetName(const char* value);

	// Volume
	float GetVolume();
	void SetVolume(float value);

	// IsPlaying
	bool GetIsPlaying();
	void SetIsPlaying(bool value);

	// IsLooping
	bool GetIsLooping();
	void SetIsLooping(bool value);

	// PlayPastEnd
	bool GetPlayPastEnd();
	void SetPlayPastEnd(bool value);

	// AppendRemove
	bool GetAppendRemove();
	void SetAppendRemove(bool value);

	// HasBeenRemoved
	bool GetHasBeenRemoved();
	bool GetHasReachedEnd();

	// OnFinishedAction
	AudioFinishedAction GetOnFinishedAction();
	void SetOnFinishedAction(AudioFinishedAction value);

	// SamplePosition
	int64_t GetSamplePosition();
	void SetSamplePosition(int64_t value);

	inline int64_t GetSampleCount();
	inline uint32_t GetSampleRate();
	inline uint32_t GetChannelCount();

private:
	// Members Variables
	// -----------------
	const char* name = "NO-NAME";

	float volume = MAX_VOLUME;

	bool isPlaying = false;
	bool isLooping = false;
	bool playPastEnd = false;
	bool hasBeenRemoved = false;
	bool appendRemove = false;

	int64_t samplePosition = 0;
	AudioFinishedAction onFinishedAction = AUDIO_FINISHED_NONE;
	ISampleProvider* sampleProvider = nullptr;
	// -----------------

protected:
	// Used by AudioEngine
	// -------------------
	inline void SetHasBeenRemoved(bool value) { hasBeenRemoved = value; };
	inline void IncrementSamplePosition(int64_t value) { SetSamplePosition(GetSamplePosition() + value); };

public:
	// Conversion Helper Methods
	// -------------------------
	TimeSpan SamplesToTimeSpan(double samples);
	int64_t TimeSpanToSamples(TimeSpan time);

protected:
	// Conversion Helper Functions
	// ---------------------------
	static TimeSpan SamplesToTimeSpan(double samples, double sampleRate, double channelCount);
	static int64_t TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount);
};