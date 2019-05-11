#pragma once
#include "ISampleProvider.h"
#include <string>

class MemoryAudioStream : public ISampleProvider
{
public:
	MemoryAudioStream();
	MemoryAudioStream(const std::string& filePath);
	~MemoryAudioStream();

	void LoadFromFile(const std::string& filePath);
	void Dispose();

	virtual size_t ReadSamples(int16_t* bufferToFill, size_t sampleOffset, size_t samplesToRead) override;
	virtual size_t GetSampleCount() override;

	virtual uint32_t GetChannelCount() override;
	virtual uint32_t GetSampleRate() override;

	inline bool GetIsInitialized() { return initialized; };
	inline int16_t* GetSampleData() { return sampleData; };

private:
	bool initialized = false;

	uint32_t channelCount;
	uint32_t sampleRate;

	size_t sampleCount = 0;
	int16_t* sampleData = nullptr;

	uint32_t GetFileMagic(const wchar_t* filePath);
	void LoadWave(const std::wstring& filePath);
	void LoadFlac(const std::wstring& filePath);
	void LoadOgg(const std::wstring& filePath);
	void LoadMp3(const std::wstring& filePath);
};
