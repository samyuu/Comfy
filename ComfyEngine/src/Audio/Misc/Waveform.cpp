#include "Waveform.h"

namespace Comfy::Audio
{
	void Waveform::Calculate(ISampleProvider& sampleProvider, TimeSpan timePerPixel)
	{
		const size_t sampleCount = sampleProvider.GetFrameCount() * sampleProvider.GetChannelCount();
		const u32 sampleRate = sampleProvider.GetSampleRate();
		const size_t channelCount = sampleProvider.GetChannelCount();
		const i16* sampleData = sampleProvider.GetRawSampleView();

		std::unique_ptr<i16[]> tempOwningSampleBuffer = nullptr;
		if (sampleData == nullptr)
		{
			tempOwningSampleBuffer = std::make_unique<i16[]>(sampleCount);
			sampleData = tempOwningSampleBuffer.get();
			
			sampleProvider.ReadSamples(tempOwningSampleBuffer.get(), 0, sampleProvider.GetFrameCount(), sampleProvider.GetChannelCount());
		}

		// TODO: This should probably be calculated as floating points
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

	void Waveform::Clear()
	{
		pixelPCMs.clear();
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
