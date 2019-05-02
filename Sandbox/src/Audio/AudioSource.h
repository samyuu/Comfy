#pragma once
#include "ISampleProvider.h"
#include "AudioEngine.h"

class AudioSource
{
public:
	AudioSource(ISampleProvider* sampleProvider);
	~AudioSource();
	
	inline ISampleProvider* GetSampleProvider() { return sampleProvider; };

	inline float GetVolume() { return volume; };
	inline void SetVolume(float value) { volume = std::clamp(value, MIN_VOLUME, MAX_VOLUME); };

private:
	ISampleProvider* sampleProvider = nullptr;
	float volume = MAX_VOLUME;
};