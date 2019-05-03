#pragma once
#include "ISampleProvider.h"

class MemoryAudioStream : public ISampleProvider
{
public:
	MemoryAudioStream();
	~MemoryAudioStream();

	void LoadFromFile(const char* filePath);
	void Dispose();

	virtual size_t ReadSamples(int16_t* bufferToFill, size_t sampleOffset, size_t samplesToRead) override;
	virtual size_t GetSampleCount() override;

	virtual uint32_t GetChannelCount() override;
	virtual uint32_t GetSampleRate() override;

	inline bool GetIsInitialized() { return initialized; };

private:
	bool initialized = false;

	uint32_t channelCount;
	uint32_t sampleRate;

	size_t sampleCount = 0;
	int16_t* sampleData = nullptr;
};

