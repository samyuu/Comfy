/******************************************/
/*
  playraw.cpp
  by Gary P. Scavone, 2007

  Play a specified raw file.  It is necessary
  that the file be of the same data format as
  defined below.
*/
/******************************************/

#include "RtAudio.h"
#include <windows.h>
#include <assert.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <stdio.h>
#include <chrono>
#include <thread>
#include <sndfile.h>

typedef unsigned __int8 byte;

typedef int16_t SampleDataType;
#define FORMAT RTAUDIO_SINT16

struct RawAudioFile
{
	//uint64_t BytePosition;
	uint64_t ByteLength;
	byte* RawSongData;
};

RawAudioFile uchuuSamurai;
RawAudioFile coldOne;

uint64_t SongBytePosition;
//uint64_t SongByteLength;
//byte* RawSongDataByte = nullptr;
uint32_t Channels = 0;
uint32_t SampleRate = 0;

bool KeyStates[255], LastKeyStates[255];

int coldOneOffset = -1; // 347136 + 80000;

byte* ReadAllBytes(const char* path, uint64_t* fileSize)
{
	FILE* file = nullptr;
	fopen_s(&file, path, "rb");

	assert(file);

	fseek(file, 0, SEEK_END);
	*fileSize = ftell(file);
	fseek(file, 0, SEEK_SET);

	byte* allBytes = new byte[*fileSize];
	size_t bytesRead = fread(allBytes, sizeof(byte), *fileSize, file);

	fclose(file);

	assert(bytesRead == *fileSize);
	return allBytes;
}

RawAudioFile ReadRawAudioFile(const char* path)
{
	RawAudioFile audioFile;
	audioFile.RawSongData = ReadAllBytes(path, &audioFile.ByteLength);
	return audioFile;
}

void SleepPrecise(double dt)
{
	typedef std::chrono::high_resolution_clock clock;
	using duration = std::chrono::duration<double>;

	static constexpr duration MinSleepDuration(0);
	clock::time_point start = clock::now();

	while (duration(clock::now() - start).count() < (dt / 1000.0))
		std::this_thread::sleep_for(MinSleepDuration);
}

inline int16_t MixSamples(int16_t a, int16_t b)
{
	return
		// If both samples are negative, mixed signal must have an amplitude between the lesser of A and B, and the minimum permissible negative amplitude
		a < 0 && b < 0 ?
		((int)a + (int)b) - (((int)a * (int)b) / INT16_MIN) :
		// If both samples are positive, mixed signal must have an amplitude between the greater of A and B, and the maximum permissible positive amplitude
		(a > 0 && b > 0 ?
		((int)a + (int)b) - (((int)a * (int)b) / INT16_MAX)
			// If samples are on opposite sides of the 0-crossing, mixed signal should reflect that samples cancel each other out somewhat
			:
			a + b);
}

int AudioCallback(void* outputBuffer, void* inputBuffer, unsigned int bufferFrames, double streamTime, RtAudioStreamStatus status, void* data)
{
	//printf("streamTime: %f\n", streamTime);

	//AdditionalCallbackData *additionalData = (AdditionalCallbackData*)data;
	byte* audioDataBuffer = (byte*)outputBuffer;

	size_t frameSize = Channels * sizeof(SampleDataType);	// 4
	size_t elementCount = bufferFrames;						// 64
	size_t frameStride = frameSize * elementCount;			// 256

	ZeroMemory(audioDataBuffer, frameStride);
	//memcpy(audioDataBuffer, uchuuSamurai.RawSongData + 0, elementStride);
	memcpy(audioDataBuffer, uchuuSamurai.RawSongData + SongBytePosition, frameStride);
	SongBytePosition += frameStride;

	short* audioBufferShorts = (short*)audioDataBuffer;

	struct AudioFrame
	{
		union
		{
			short Channels[2];
			struct
			{
				short LeftChannel;
				short RightChannel;
			};
		};
	};

	AudioFrame* audioBufferFrames = (AudioFrame*)audioDataBuffer;

	//for (size_t i = 0; i < nBufferFrames; i++)
	//	audioBufferFrames[i].RightChannel = audioBufferFrames[i].LeftChannel;

	//for (size_t i = 0; i < elementCount * Channels; i += 2)
	//{
	//	//audioBufferShorts[i] = 0;
	//	//audioBufferShorts[i + 1] = audioBufferShorts[i];
	//	audioBufferShorts[i] = audioBufferShorts[i + 1];
	//}

	if (coldOneOffset > -1 && SongBytePosition > coldOneOffset && SongBytePosition + frameStride < coldOne.ByteLength + coldOneOffset)
	{
		short* coldOneShorts = (short*)(&coldOne.RawSongData[(SongBytePosition - coldOneOffset) / 2]);

		AudioFrame* coldOneFrames = (AudioFrame*)alloca(bufferFrames * sizeof(AudioFrame));
		
		for (size_t i = 0; i < bufferFrames; i++)
		{
			coldOneFrames[i].LeftChannel = coldOneShorts[i];
			coldOneFrames[i].RightChannel = coldOneShorts[i];
		}

		coldOneShorts = (short*)coldOneFrames;
		for (size_t i = 0; i < frameStride / Channels; i++)
		{
			short a = audioBufferShorts[i];
			short b = coldOneShorts[i];
			//b *= .75f;
			audioBufferShorts[i] = MixSamples(a, b);
		}
	}

	if (SongBytePosition + frameStride >= uchuuSamurai.ByteLength)
	{
		//SongBytePosition = 0;
		return 0; // 1;
	}

	// if end of file
	//if (count < elementCount)
	//{
	//	size_t bytesCount = (elementCount - count) * elementSize;
	//	size_t startByte = count * elementSize;

	//	ZeroMemory(audioDataBuffer + startByte, bytesCount);

	//	return 1; // stop callback
	//}

	return 0; // all OK
}

int main(int argc, char *argv[])
{
	bool asio = !true;
	RtAudio rtAudio = RtAudio(asio ? RtAudio::WINDOWS_ASIO : RtAudio::WINDOWS_WASAPI);

	assert(rtAudio.getDeviceCount() >= 1);

	Channels = atoi(argv[1]);
	SampleRate = atoi(argv[2]);

	char* filePath = argv[3];
	//uchuuSamurai = ReadRawAudioFile(filePath);
	//coldOne = ReadRawAudioFile("rom/nop_cracking_open_a_cold_one.raw");
	//coldOne = ReadRawAudioFile("rom/01_button1.raw");

	if (true)
	{
		SF_INFO sf_info = {};
		//SNDFILE *snd_file = sf_open("rom/selector_vox_lp.ogg", SFM_READ, &sf_info);
		SNDFILE *snd_file = sf_open("rom/button/01_button1.wav", SFM_READ, &sf_info);
		//SNDFILE *snd_file = sf_open("rom/button/24_tambourine_2nd.wav", SFM_READ, &sf_info);
		assert(snd_file);

		size_t totalFrames = sf_info.frames * sf_info.channels;

		short* rawShortData = new short[totalFrames];
		size_t read_count = sf_readf_short(snd_file, rawShortData, sf_info.frames);

		sf_close(snd_file);

		coldOne.ByteLength = read_count * sizeof(short) * Channels;
		coldOne.RawSongData = (byte*)rawShortData;
	}

	{
		SF_INFO sf_info = {};
		//SNDFILE *snd_file = sf_open("rom/selector_vox_lp.ogg", SFM_READ, &sf_info);
		//SNDFILE *snd_file = sf_open("rom/07 アザレア.flac", SFM_READ, &sf_info);
		//SNDFILE *snd_file = sf_open("rom/samurai.wav", SFM_READ, &sf_info);
		SNDFILE *snd_file = sf_open("D:/Nop/音楽/VALLEYSTONE/懐色坂/06 Butterfly Effect feat. Kanata.N.flac", SFM_READ, &sf_info);
		assert(snd_file);

		size_t totalFrames = sf_info.frames * sf_info.channels;

		short* rawShortData = new short[totalFrames];
		size_t read_count = sf_readf_short(snd_file, rawShortData, sf_info.frames);

		sf_close(snd_file);

		uchuuSamurai.ByteLength = read_count * sizeof(short) * Channels;
		uchuuSamurai.RawSongData = (byte*)rawShortData;
	}


	//RawSongDataByte = ReadAllBytes(filePath, &SongByteLength);

	// Set our stream parameters for output only.
	RtAudio::StreamParameters streamParameters;
	streamParameters.deviceId = rtAudio.getDefaultOutputDevice();
	streamParameters.nChannels = Channels;
	streamParameters.firstChannel = 0;

	uint32_t bufferSize = 64;

	constexpr auto isTapped = [](byte key) { return KeyStates[key] && !LastKeyStates[key]; };

	try
	{
		//rtAudio.openStream(&streamParameters, NULL, FORMAT, sampleRate, &bufferSize, &AudioCallback, (void*)&outputData);
		rtAudio.openStream(&streamParameters, NULL, FORMAT, SampleRate, &bufferSize, &AudioCallback, nullptr);
		rtAudio.startStream();
	}
	catch (RtAudioError& e)
	{
		fprintf(stderr, "%s\n", e.getMessage().c_str());
		goto cleanup;
	}

	printf("Playing raw file: %s (buffer size = %d)\n", filePath, bufferSize);

	while (true)
	{
		for (unsigned char i = 0; i < sizeof(KeyStates); i++)
		{
			LastKeyStates[i] = KeyStates[i];
			KeyStates[i] = GetAsyncKeyState(i);
		}

		if (GetAsyncKeyState(VK_RIGHT))
			SongBytePosition = min(SongBytePosition + SampleRate, uchuuSamurai.ByteLength);
		if (GetAsyncKeyState(VK_LEFT))
			SongBytePosition = min(SongBytePosition - SampleRate, 0);

		char keys[] = { 'W', 'A', 'S', 'D', 'I', 'J', 'K', 'L' };

		for (char key = 0; key < sizeof(keys); key++)
		{
			if (isTapped(keys[key]))
				coldOneOffset = SongBytePosition;
		}

		if (!rtAudio.isStreamRunning() || GetAsyncKeyState(VK_ESCAPE))
			break;

		//printf("position: %d / %d\n", SongBytePosition, uchuuSamurai.ByteLength);

		if (GetAsyncKeyState(VK_UP))
		{
			SleepPrecise(1000);
			SongBytePosition -= SampleRate * Channels * Channels;
		}
		else
		{
			constexpr double frameRate = 300.0;
			double sleepDuration = 1000.0 / frameRate;
			//SleepPrecise(sleepDuration);

			//Sleep(1);
		}
	}

cleanup:
	delete[] uchuuSamurai.RawSongData;

	rtAudio.closeStream();
	return 0;
}