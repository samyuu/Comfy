#pragma once
#include "Types.h"

class ISampleProvider
{
public:
	virtual int64_t ReadSamples(int16_t* bufferToFill, int64_t sampleOffset, int64_t samplesToRead) = 0;
	virtual int64_t GetSampleCount() = 0;

	virtual uint32_t GetChannelCount() = 0;
	virtual uint32_t GetSampleRate() = 0;
};