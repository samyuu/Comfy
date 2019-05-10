#pragma once
#include "MemoryAudioStream.h"
#include <vector>

class TimeSpan;

class Waveform
{
public:
	Waveform();
	~Waveform();

	void Calculate(MemoryAudioStream* audioStream, TimeSpan timePerPixel);
	float GetPcmForPixel(size_t pixel);
	size_t GetPixelCount();

protected:
	// Mapping of pixel <-> averaged PCM
	std::vector<float> pixelPCMs;
};
