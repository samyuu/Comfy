#pragma once
#include "ISampleProvider.h"
#include "AudioEngine.h"
#include "TimeSpan.h"
#include <string>

enum class AudioFinishedAction : int8_t
{
	None,
	Remove,
	Count,
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
	ISampleProvider* GetSampleProvider() const;
	void SetSampleProvider(ISampleProvider* provider);
	bool IsSampleProviderValid();

	// Position
	TimeSpan GetPosition() const;
	void SetPosition(TimeSpan value);
	void Restart();

	// Duration
	TimeSpan GetDuration() const;

	// Name
	const char* GetName() const;
	void SetName(const char* value);

	// Volume
	float GetVolume() const;
	void SetVolume(float value);

	// IsPlaying
	bool GetIsPlaying() const;
	void SetIsPlaying(bool value);

	// IsLooping
	bool GetIsLooping() const;
	void SetIsLooping(bool value);

	// PlayPastEnd
	bool GetPlayPastEnd() const;
	void SetPlayPastEnd(bool value);

	// AppendRemove
	bool GetAppendRemove() const;
	void SetAppendRemove(bool value);

	// HasBeenRemoved
	bool GetHasBeenRemoved() const;
	bool GetHasReachedEnd() const;

	// OnFinishedAction
	AudioFinishedAction GetOnFinishedAction() const;
	void SetOnFinishedAction(AudioFinishedAction value);

	// SamplePosition
	int64_t GetSamplePosition() const;
	void SetSamplePosition(int64_t value);

	inline int64_t GetSampleCount() const;
	inline uint32_t GetSampleRate() const;
	inline uint32_t GetChannelCount() const;

private:
	// Members Variables
	// -----------------
	const char* name = "<NO-NAME>";

	float volume = MAX_VOLUME;

	bool isPlaying = false;
	bool isLooping = false;
	bool playPastEnd = false;
	bool hasBeenRemoved = false;
	bool appendRemove = false;

	int64_t samplePosition = 0;
	AudioFinishedAction onFinishedAction = AudioFinishedAction::None;
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
	TimeSpan SamplesToTimeSpan(double samples) const;
	int64_t TimeSpanToSamples(TimeSpan time) const;

protected:
	// Conversion Helper Functions
	// ---------------------------
	static TimeSpan SamplesToTimeSpan(double samples, double sampleRate, double channelCount);
	static int64_t TimeSpanToSamples(TimeSpan time, double sampleRate, double channelCount);
};