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
	size_t samplesPerPixel = timePerPixel.TotalSeconds() * sampleRate * channelCount;

	// average all samples in that pixel range
	size_t totalPixels = sampleCount / samplesPerPixel;
	pixelPCMs.resize(totalPixels);

	for (size_t pixel = 0; pixel < totalPixels; pixel++)
	{
		int64_t totalPcm = 0;

		for (size_t i = 0; i < samplesPerPixel; i += channelCount)
		{
			int16_t shortPcm = sampleData[(pixel * samplesPerPixel) + i];
			totalPcm += abs(shortPcm);
		}

		int64_t averagePcm = totalPcm / samplesPerPixel / channelCount;
		float floatPcm = averagePcm / (double)SHRT_MAX / 2.0;

		pixelPCMs[pixel] = floatPcm;
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
