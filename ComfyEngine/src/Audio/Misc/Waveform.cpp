#include "Waveform.h"
#include "Audio/Core/Resample.h"

namespace Comfy::Audio
{
	namespace
	{
		constexpr float AveragePCMAtPixel(double atPixel, u32 atChannel, const i16* samples, size_t sampleCount, f64 sampleRate, u32 channelCount, f64 secondsPerPixel)
		{
			const f64 pixelStartTime = (atPixel * secondsPerPixel);
			const f64 pixelEndTime = ((atPixel + 1.0) * secondsPerPixel);

			const f64 secondsPerSample = 1.0 / sampleRate;

			i32 pcmSum = 0, pcmCount = 0;
			for (f64 time = pixelStartTime; time < pixelEndTime; time += secondsPerSample)
			{
				pcmSum += glm::abs(SampleAtTime<i16, Interpolation::Linear>(time, atChannel, samples, sampleCount, sampleRate, channelCount));
				pcmCount++;
			}

			const i32 averagePCM = (pcmSum / pcmCount);
			const f32 normalizedPCM = averagePCM / static_cast<f32>(std::numeric_limits<i16>::max());

			return normalizedPCM;
		}
	}

	void Waveform::Calculate(ISampleProvider& sampleProvider, TimeSpan timePerPixel)
	{
		const auto frameCount = sampleProvider.GetFrameCount();
		channelCount = sampleProvider.GetChannelCount();
		sampleCount = (frameCount * channelCount);
		sampleRate = static_cast<f64>(sampleProvider.GetSampleRate());
		secondsPerPixel = timePerPixel.TotalSeconds();

		if (&sampleProvider != lastSampleProvider)
		{
			sampleDataCopy = std::make_unique<i16[]>(sampleCount);
			sampleProvider.ReadSamples(sampleDataCopy.get(), 0, frameCount, channelCount);

			lastSampleProvider = &sampleProvider;
		}

		const f64 samplesPerPixel = (secondsPerPixel * sampleRate * channelCount);
		const auto newPixelCount = static_cast<size_t>(sampleCount / samplesPerPixel);

		if (cachedPixelPCMs == nullptr || newPixelCount > pixelCount)
		{
			cachedPixelPCMs = std::make_unique<f32[]>(newPixelCount);
			cachedPixelBits.resize(newPixelCount);
		}

		std::fill(cachedPixelBits.begin(), cachedPixelBits.end(), 0);

		pixelCount = newPixelCount;
	}

	void Waveform::Clear()
	{
		cachedPixelPCMs = nullptr;
		pixelCount = 0;
	}

	float Waveform::GetPCMForPixel(i64 pixel)
	{
		assert(pixel >= 0 && static_cast<size_t>(pixel) < pixelCount);

		if (cachedPixelBits[pixel])
			return cachedPixelPCMs[pixel];

		float result;
		if (channelCount >= 2)
		{
			// NOTE: Purposly ignore all but the first two channels to avoid a potentially noisy waveform and improve performance slightly
			for (size_t c = 0; c < 2; c++)
				result =+ AveragePCMAtPixel(static_cast<double>(pixel), c, sampleDataCopy.get(), sampleCount, sampleRate, channelCount, secondsPerPixel);
			result /= 2.0f;
		}
		else
		{
			result = AveragePCMAtPixel(static_cast<double>(pixel), 0, sampleDataCopy.get(), sampleCount, sampleRate, channelCount, secondsPerPixel);
		}

		cachedPixelBits[pixel] = true;
		cachedPixelPCMs[pixel] = result;

		return result;
	}

	size_t Waveform::GetPixelCount() const
	{
		return pixelCount;
	}
}
