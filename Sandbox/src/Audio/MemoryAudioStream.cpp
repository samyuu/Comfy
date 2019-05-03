#include "MemoryAudioStream.h"
#include <sndfile.h>
#include <assert.h>
#include <memory>

MemoryAudioStream::MemoryAudioStream()
{
}

MemoryAudioStream::~MemoryAudioStream()
{
}

void MemoryAudioStream::LoadFromFile(const char* filePath)
{
	assert(!GetIsInitialized());

	SF_INFO sfInfo = {};
	SNDFILE *sndFile = sf_open(filePath, SFM_READ, &sfInfo);
	{
		assert(sndFile);

		channelCount = sfInfo.channels;
		sampleRate = sfInfo.samplerate;
		sampleCount = sfInfo.frames * sfInfo.channels;
		sampleData = new int16_t[sampleCount];

		size_t readCount = sf_readf_short(sndFile, sampleData, sfInfo.frames);
		size_t samplesRead = readCount * channelCount;

		assert(samplesRead == sampleCount);
	}
	sf_close(sndFile);

	initialized = true;
}

void MemoryAudioStream::Dispose()
{
	if (sampleData != nullptr)
	{
		delete sampleData;
		sampleData = nullptr;
	}
}

size_t MemoryAudioStream::ReadSamples(int16_t* bufferToFill, size_t sampleOffset, size_t samplesToRead)
{
	int16_t* sampleSource = &sampleData[sampleOffset];
	size_t samplesRead = (sampleOffset + samplesToRead > GetSampleCount()) ? GetSampleCount() - sampleOffset : samplesToRead;

	memcpy(bufferToFill, sampleSource, samplesRead * sizeof(int16_t));
	return samplesRead;
}

size_t MemoryAudioStream::GetSampleCount()
{
	return sampleCount;
}

uint32_t MemoryAudioStream::GetChannelCount()
{
	return channelCount;
}

uint32_t MemoryAudioStream::GetSampleRate()
{
	return sampleRate;
}