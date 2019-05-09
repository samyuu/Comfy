#include "DummySampleProvider.h"

size_t DummySampleProvider::ReadSamples(int16_t* bufferToFill, size_t sampleOffset, size_t samplesToRead)
{
	return 0;
}

size_t DummySampleProvider::GetSampleCount()
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