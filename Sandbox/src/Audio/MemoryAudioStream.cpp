#include "MemoryAudioStream.h"
#include "ChannelMixer.h"
#include "AudioEngine.h"
#include "Decoder/dr_wav.h"
#include "Decoder/dr_flac.h"
#include "Decoder/dr_mp3.h"
#include <assert.h>
#include <windows.h>

MemoryAudioStream::MemoryAudioStream()
{
}

MemoryAudioStream::MemoryAudioStream(const std::string& filePath)
{
	LoadFromFile(filePath);
}

MemoryAudioStream::~MemoryAudioStream()
{
}

void MemoryAudioStream::LoadFromFile(const std::string& filePath)
{
	assert(!GetIsInitialized());

	wchar_t widePath[MAX_PATH];
	int wideBytes = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filePath.c_str(), -1, widePath, sizeof(widePath));

	uint32_t fileMagic = GetFileMagic(widePath);
	uint32_t swappedMagic = ((fileMagic >> 24) & 0xff) | ((fileMagic << 8) & 0xff0000) | ((fileMagic >> 8) & 0xff00) |  ((fileMagic << 24) & 0xff000000); 

	switch (swappedMagic)
	{
	case 'RIFF':
	case 'WAVE':
		LoadWave(widePath);
		break;

	case 'fLaC':
		LoadFlac(widePath);
		break;

	case 'OggS':
		LoadOgg(widePath);
		break;

	case 'ID3' + (0x3 >> 24):
		LoadMp3(widePath);
		break;

	default:
		assert(false);
		return;
	}

	uint32_t channelTargetCount = AudioEngine::GetInstance()->GetChannelCount();
	if (this->channelCount != channelTargetCount)
	{
		ChannelMixer mixer(channelTargetCount);
		mixer.MixChannels(&sampleData, &sampleCount, &channelCount);
	}

	initialized = true;
}

void MemoryAudioStream::Dispose()
{
	if (sampleData != nullptr)
	{
		delete[] sampleData;
		sampleData = nullptr;
	}

	initialized = false;
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

uint32_t MemoryAudioStream::GetFileMagic(const wchar_t* filePath)
{
	FILE* file;
	_wfopen_s(&file, filePath, L"r");
	
	uint32_t magic = NULL;
	fread(&magic, sizeof(magic), 1, file);
	
	fclose(file);
	return magic;
}

void MemoryAudioStream::LoadWave(const std::wstring& filePath)
{
	this->sampleData = drwav_open_file_and_read_pcm_frames_s16(filePath.c_str(), &channelCount, &sampleRate, &sampleCount);
	sampleCount *= channelCount;
}

void MemoryAudioStream::LoadFlac(const std::wstring& filePath)
{
	this->sampleData = drflac_open_file_and_read_pcm_frames_s16(filePath.c_str(), &channelCount, &sampleRate, &sampleCount);
	sampleCount *= channelCount;
}

void MemoryAudioStream::LoadOgg(const std::wstring& filePath)
{
	assert(false);
}

void MemoryAudioStream::LoadMp3(const std::wstring& filePath)
{
	drmp3_config config;
	config.outputChannels = AudioEngine::GetInstance()->GetChannelCount();
	config.outputSampleRate = AudioEngine::GetInstance()->GetSampleRate();

	uint64_t frameCount = NULL;
	this->sampleData = drmp3_open_file_and_read_s16(filePath.c_str(), &config, &frameCount);

	channelCount = config.outputChannels;
	sampleRate = config.outputSampleRate;
	sampleCount = frameCount * channelCount;
}