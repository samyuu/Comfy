#pragma once
#include "ISampleProvider.h"

class DummySampleProvider : public ISampleProvider
{
public:
	virtual size_t ReadSamples(int16_t* bufferToFill, size_t sampleOffset, size_t samplesToRead) override;
	virtual size_t GetSampleCount() override;

	virtual uint32_t GetChannelCount() override;
	virtual uint32_t GetSampleRate() override;
};