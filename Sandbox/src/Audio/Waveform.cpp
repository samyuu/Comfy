#include "Waveform.h"
#include "Core/TimeSpan.h"
#include "System/BuildConfiguration.h"

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
	int64_t samplesPerPixel = static_cast<int64_t>(timePerPixel.TotalSeconds() * sampleRate * channelCount);
	int64_t samplesPerChannelPixel = static_cast<int64_t>(samplesPerPixel / channelCount);

	int64_t totalPixels = sampleCount / samplesPerPixel;
	
	//pixelPCMs.resize(totalPixels);
	pixelPCMs.clear();
	pixelPCMs.reserve(totalPixels);

	for (int64_t pixel = 0; pixel < totalPixels; pixel++)
	{
		// calculate the average of all samples in the per pixel range
		int64_t totalPcm = 0;

		for (int64_t i = 0; i < samplesPerPixel; i += channelCount)
		{
			int16_t shortPcm = sampleData[(pixel * samplesPerPixel) + i];
			totalPcm += glm::abs(shortPcm);
		}

		int64_t averagePcm = totalPcm / samplesPerChannelPixel;

		float floatPcm = averagePcm / static_cast<float>(std::numeric_limits<int16_t>::max());
		pixelPCMs.push_back(floatPcm);
	}
}

float Waveform::GetPcmForPixel(int64_t pixel)
{
	return DEBUG_RELEASE_SWITCH(pixelPCMs.at(pixel), pixelPCMs[pixel]);
}

size_t Waveform::GetPixelCount()
{
	return pixelPCMs.size();
}
