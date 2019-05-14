#include "DummySampleProvider.h"
#include <cstring>

int64_t DummySampleProvider::ReadSamples(int16_t* bufferToFill, int64_t sampleOffset, int64_t samplesToRead)
{
	memset(bufferToFill, 0, samplesToRead * sizeof(int16_t));
	return samplesToRead;
}

int64_t DummySampleProvider::GetSampleCount()
{
	return 0;
};

uint32_t DummySampleProvider::GetChannelCount()
{
	return 2;
}

uint32_t DummySampleProvider::GetSampleRate()
{
	return 44100;
}