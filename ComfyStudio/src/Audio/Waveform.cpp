#include "Waveform.h"
#include "System/Version/BuildConfiguration.h"

namespace Comfy::Audio
{
	Waveform::Waveform()
	{
	}

	Waveform::~Waveform()
	{
	}

	void Waveform::Calculate(MemorySampleProvider* audioStream, TimeSpan timePerPixel)
	{
		const int16_t* sampleData = audioStream->GetSampleData();
		const size_t sampleCount = audioStream->GetFrameCount() * audioStream->GetChannelCount();
		const uint32_t sampleRate = audioStream->GetSampleRate();
		const size_t channelCount = audioStream->GetChannelCount();

		// NOTE: Calculate how many samples per pixel at the current zoom
		const int64_t samplesPerPixel = static_cast<int64_t>(timePerPixel.TotalSeconds() * sampleRate * channelCount);
		const int64_t samplesPerChannelPixel = static_cast<int64_t>(samplesPerPixel / channelCount);

		const int64_t totalPixels = sampleCount / samplesPerPixel;

		// pixelPCMs.resize(totalPixels);
		pixelPCMs.clear();
		pixelPCMs.reserve(totalPixels);

		for (int64_t pixel = 0; pixel < totalPixels; pixel++)
		{
			// NOTE: Calculate the average of all samples in the per pixel range
			int64_t totalPcm = 0;

			for (int64_t i = 0; i < samplesPerPixel; i += channelCount)
			{
				const int16_t shortPcm = sampleData[(pixel * samplesPerPixel) + i];
				totalPcm += glm::abs(shortPcm);
			}

			const int64_t averagePcm = totalPcm / samplesPerChannelPixel;
			const float floatPcm = averagePcm / static_cast<float>(std::numeric_limits<int16_t>::max());
			
			pixelPCMs.push_back(floatPcm);
		}
	}

	float Waveform::GetPcmForPixel(int64_t pixel) const
	{
		return DEBUG_RELEASE_SWITCH(pixelPCMs.at(pixel), pixelPCMs[pixel]);
	}

	size_t Waveform::GetPixelCount() const
	{
		return pixelPCMs.size();
	}
}
