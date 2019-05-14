#pragma once
#include "ISampleProvider.h"

class DummySampleProvider : public ISampleProvider
{
public:
	virtual int64_t ReadSamples(int16_t* bufferToFill, int64_t sampleOffset, int64_t samplesToRead) override;
	virtual int64_t GetSampleCount() override;

	virtual uint32_t GetChannelCount() override;
	virtual uint32_t GetSampleRate() override;
};