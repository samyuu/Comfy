#pragma once
#include "MemoryAudioStream.h"
#include <vector>

struct TimeSpan;

class Waveform
{
public:
	Waveform();
	~Waveform();

	void Calculate(MemoryAudioStream* audioStream, TimeSpan timePerPixel);
	float GetPcmForPixel(int64_t pixel);
	size_t GetPixelCount();

protected:
	// Mapping of pixel <-> averaged PCM
	std::vector<float> pixelPCMs;
};
