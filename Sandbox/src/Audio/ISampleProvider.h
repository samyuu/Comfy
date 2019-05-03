#pragma once
#include <stdint.h>

class ISampleProvider
{
public:
	virtual size_t ReadSamples(int16_t* bufferToFill, size_t sampleOffset, size_t samplesToRead) = 0;
	virtual size_t GetSampleCount() = 0;

	virtual uint32_t GetChannelCount() = 0;
	virtual uint32_t GetSampleRate() = 0;
};