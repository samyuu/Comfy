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
		const i16* sampleData = audioStream->GetSampleData();
		const size_t sampleCount = audioStream->GetFrameCount() * audioStream->GetChannelCount();
		const u32 sampleRate = audioStream->GetSampleRate();
		const size_t channelCount = audioStream->GetChannelCount();

		// NOTE: Calculate how many samples per pixel at the current zoom
		const i64 samplesPerPixel = static_cast<i64>(timePerPixel.TotalSeconds() * sampleRate * channelCount);
		const i64 samplesPerChannelPixel = static_cast<i64>(samplesPerPixel / channelCount);

		const i64 totalPixels = sampleCount / samplesPerPixel;

		// pixelPCMs.resize(totalPixels);
		pixelPCMs.clear();
		pixelPCMs.reserve(totalPixels);

		for (i64 pixel = 0; pixel < totalPixels; pixel++)
		{
			// NOTE: Calculate the average of all samples in the per pixel range
			i64 totalPcm = 0;

			for (i64 i = 0; i < samplesPerPixel; i += channelCount)
			{
				const i16 shortPcm = sampleData[(pixel * samplesPerPixel) + i];
				totalPcm += glm::abs(shortPcm);
			}

			const i64 averagePcm = totalPcm / samplesPerChannelPixel;
			const float floatPcm = averagePcm / static_cast<float>(std::numeric_limits<i16>::max());
			
			pixelPCMs.push_back(floatPcm);
		}
	}

	float Waveform::GetPcmForPixel(i64 pixel) const
	{
		return pixelPCMs[pixel];
	}

	size_t Waveform::GetPixelCount() const
	{
		return pixelPCMs.size();
	}
}
