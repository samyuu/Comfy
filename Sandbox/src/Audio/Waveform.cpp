#include "Waveform.h"
#include "../TimeSpan.h"

Waveform::Waveform()
{
}

Waveform::~Waveform()
{
}

void Waveform::Calculate(MemoryAudioStream* audioStream, TimeSpan timePerPixel)
{
	int16_t* sampleData = audioStream->GetSampleData();
	size_t sampleCount = audioStream->GetSampleCount();
	uint32_t sampleRate = audioStream->GetSampleRate();
	size_t channelCount = audioStream->GetChannelCount();

	// calculate how many samples per pixel at the current zoom
	int64_t samplesPerPixel = timePerPixel.TotalSeconds() * sampleRate * channelCount;
	int64_t samplesPerChannelPixel = (samplesPerPixel / channelCount);

	int64_t totalPixels = sampleCount / samplesPerPixel;
	
	//pixelPCMs.resize(totalPixels);
	pixelPCMs.clear();
	pixelPCMs.reserve(totalPixels);

	for (int64_t pixel = 0; pixel < totalPixels; pixel++)
	{
		// calculate the average of all samples in the per pixel range
		int64_t totalPcm = 0;

		for (size_t i = 0; i < samplesPerPixel; i += channelCount)
		{
			int16_t shortPcm = sampleData[(pixel * samplesPerPixel) + i];
			totalPcm += abs(shortPcm);
		}

		int64_t averagePcm = totalPcm / samplesPerChannelPixel;

		float floatPcm = averagePcm / (float)SHRT_MAX;
		pixelPCMs.push_back(floatPcm);
	}
}

float Waveform::GetPcmForPixel(size_t pixel)
{
	return pixelPCMs.at(pixel);
}

size_t Waveform::GetPixelCount()
{
	return pixelPCMs.size();
}
