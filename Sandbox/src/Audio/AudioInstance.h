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
	friend class AudioEngine;
	
	// Constructors
	// ------------
	AudioInstance(ISampleProvider* sampleProvider);
	AudioInstance(ISampleProvider* sampleProvider, bool playing);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction);
	AudioInstance(ISampleProvider* sampleProvider, bool playing, AudioFinishedAction finishedAction, float volume);

	// Destructors
	// -----------
	~AudioInstance();

	// Position
	TimeSpan GetPosition();
	void SetPosition(TimeSpan value);
	
	// Duration
	TimeSpan GetDuration();

	// Volume
	float GetVolume();
	void SetVolume(float value);

	// IsPlaying
	bool GetIsPlaying();
	void SetIsPlaying(bool value);

	// AppendDelete
	bool GetAppendDelete();
	void SetAppendDelete(bool value);

	// HasBeenRemoved
	bool GetHasBeenRemoved();
	bool GetHasReachedEnd();

	// OnFinishedAction
	AudioFinishedAction GetOnFinishedAction();
	void SetOnFinishedAction(AudioFinishedAction value);

	// SamplePosition
	size_t GetSamplePosition();
	void SetSamplePosition(size_t value);

	inline size_t GetSampleCount();
	inline uint32_t GetSampleRate();
	inline uint32_t GetChannelCount();

private:
	// Members Variables
	// -----------------
	float volume = MAX_VOLUME;

	bool isPlaying = false;
	bool hasBeenRemoved = false;
	bool appendDelete = false;

	size_t samplePosition = 0;
	AudioFinishedAction onFinishedAction = AUDIO_FINISHED_NONE;
	ISampleProvider* sampleProvider = nullptr;
	// -----------------

protected:
	// Used internally
	// -------------------
	inline ISampleProvider* GetSampleProvider() { return sampleProvider; };

	// Used by AudioEngine
	// -------------------
	inline void SetHasBeenRemoved(bool value) { hasBeenRemoved = value; };
	inline void IncrementSamplePosition(size_t value) { SetSamplePosition(GetSamplePosition() + value); };

	// Conversion Helper Methods
	// -------------------------
	TimeSpan SamplesToTimeSpan(double samples);
	size_t TimeSpanToSamples(TimeSpan time);

	// Conversion Helper Functions
	// ---------------------------
	static TimeSpan SamplesToTimeSpan(double samples, double sampleRate, double channelCount);
	static size_t TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount);
};