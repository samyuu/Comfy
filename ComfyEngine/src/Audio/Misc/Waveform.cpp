#include "Waveform.h"
#include "Audio/Core/Resample.h"

namespace Comfy::Audio
{
	namespace
	{
		constexpr float AveragePCMAtPixel(double atPixel, u32 atChannel, const i16* samples, size_t sampleCount, u32 sampleRate, u32 channelCount, double secondsPerPixel)
		{
			const double pixelStartTime = (atPixel * secondsPerPixel);
			const double pixelEndTime = ((atPixel + 1.0) * secondsPerPixel);

			const double secondsPerSample = 1.0 / static_cast<double>(sampleRate);

			i32 pcmSum = 0, pcmCount = 0;
			for (double time = pixelStartTime; time < pixelEndTime; time += secondsPerSample)
			{
				pcmSum += glm::abs(SampleAtTime<i16, Interpolation::Linear>(time, atChannel, samples, sampleCount, sampleRate, channelCount));
				pcmCount++;
			}

			const i32 averagePCM = (pcmSum / pcmCount);
			const float normalizedPCM = averagePCM / static_cast<float>(std::numeric_limits<i16>::max());

			return normalizedPCM;
		}
	}

	void Waveform::Calculate(ISampleProvider& sampleProvider, TimeSpan timePerPixel)
	{
		const size_t sampleCount = sampleProvider.GetFrameCount() * sampleProvider.GetChannelCount();
		const u32 sampleRate = sampleProvider.GetSampleRate();
		const u32 channelCount = sampleProvider.GetChannelCount();
		const i16* sampleData = sampleProvider.GetRawSampleView();

		std::unique_ptr<i16[]> tempOwningSampleBuffer = nullptr;
		if (sampleData == nullptr)
		{
			tempOwningSampleBuffer = std::make_unique<i16[]>(sampleCount);
			sampleData = tempOwningSampleBuffer.get();

			sampleProvider.ReadSamples(tempOwningSampleBuffer.get(), 0, sampleProvider.GetFrameCount(), sampleProvider.GetChannelCount());
		}

		const double samplesPerPixel = (timePerPixel.TotalSeconds() * sampleRate * channelCount);
		const double framesPixel = (samplesPerPixel / channelCount);

		const i64 totalPixels = static_cast<i64>(sampleCount / samplesPerPixel);

		if (pixelCount < totalPixels)
			pixelPCMs = std::make_unique<float[]>(totalPixels);
		pixelCount = totalPixels;

		for (i64 pixel = 0; pixel < totalPixels; pixel++)
			pixelPCMs[pixel] = AveragePCMAtPixel(static_cast<double>(pixel), 0, sampleData, sampleCount, sampleRate, channelCount, timePerPixel.TotalSeconds());
	}

	void Waveform::Clear()
	{
		pixelPCMs = nullptr;
		pixelCount = 0;
	}

	float Waveform::GetPcmForPixel(i64 pixel) const
	{
		assert(pixel >= 0 && pixel < pixelCount);
		return pixelPCMs[pixel];
	}

	size_t Waveform::GetPixelCount() const
	{
		return pixelCount;
	}
}
