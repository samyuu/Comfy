#pragma once
#include <stdint.h>

class ISampleProvider
{
public:
	virtual size_t GetSamplePosition() = 0;
	virtual size_t GetSampleCount() = 0;
	
	virtual size_t ReadNextSamples(int16_t* bufferToFill, size_t offset, size_t samplesToRead) = 0;
	
	inline uint32_t GetChannelCount() { return channelCount; };
	inline uint32_t GetSampleRate() { return sampleRate; };

protected:
	uint32_t channelCount;
	uint32_t sampleRate;
};