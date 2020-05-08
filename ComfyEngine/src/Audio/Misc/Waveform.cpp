#include "Waveform.h"
#include "Audio/Core/Resample.h"

namespace Comfy::Audio
{
	void Waveform::SetSource(std::shared_ptr<ISampleProvider> sampleProvider)
	{
		if (sampleProvider == nullptr || sampleProvider->GetRawSampleView() == nullptr)
		{
			source = {};
		}
		else
		{
			source.SampleProvider = sampleProvider;
			source.Samples = sampleProvider->GetRawSampleView();
			source.SampleCount = sampleProvider->GetFrameCount() * sampleProvider->GetChannelCount();
			source.ChannelCount = sampleProvider->GetChannelCount();
			source.SampleRate = static_cast<f64>(sampleProvider->GetSampleRate());
		}
	}

	void Waveform::SetScale(TimeSpan timePerPixel)
	{
		if (source.SampleProvider == nullptr)
		{
			pixelCount = 0;
			return;
		}

		secondsPerPixel = timePerPixel.TotalSeconds();
		secondsPerSample = 1.0 / (source.SampleRate);

		const auto samplesPerPixel = static_cast<f64>(secondsPerPixel * source.SampleRate * source.ChannelCount);
		const auto newPixelCount = static_cast<size_t>(source.SampleCount / samplesPerPixel);

		if (cachedPixelPCMs == nullptr || pixelCount > cachedPixelBits.size())
		{
			cachedPixelPCMs = std::make_unique<f32[]>(newPixelCount);
			cachedPixelBits.resize(newPixelCount);
		}

		std::fill(cachedPixelBits.begin(), cachedPixelBits.end(), 0);
		pixelCount = newPixelCount;
	}

	void Waveform::Clear()
	{
		source = {};
		secondsPerPixel = 0.0;
		secondsPerSample = 0.0;
		pixelCount = 0;
		cachedPixelBits.clear();
		cachedPixelBits.shrink_to_fit();
		cachedPixelPCMs = nullptr;
	}

	float Waveform::GetPCMForPixel(i64 pixel)
	{
		assert(pixel >= 0 && static_cast<size_t>(pixel) < pixelCount);

		if (cachedPixelBits[pixel])
			return cachedPixelPCMs[pixel];

		float result = 0.0f;
		if (source.ChannelCount >= 2)
		{
			// NOTE: Purposly ignore all but the first two channels to avoid a potentially noisy waveform and improve performance slightly
			for (u32 channel = 0; channel < 2; channel++)
				result += AveragePCMAtPixel(static_cast<double>(pixel), channel);
			result /= 2.0f;
		}
		else
		{
			result = AveragePCMAtPixel(static_cast<double>(pixel), 0);
		}

		cachedPixelBits[pixel] = true;
		cachedPixelPCMs[pixel] = result;

		return result;
	}

	size_t Waveform::GetPixelCount() const
	{
		return pixelCount;
	}

	float Waveform::AveragePCMAtPixel(double atPixel, u32 atChannel) const
	{
		assert(source.SampleProvider != nullptr);

		const f64 pixelStartTime = (atPixel * secondsPerPixel);
		const f64 pixelEndTime = ((atPixel + 1.0) * secondsPerPixel);

		i32 pcmSum = 0, pcmCount = 0;
		for (f64 time = pixelStartTime; time < pixelEndTime; time += secondsPerSample)
		{
			pcmSum += glm::abs(SampleAtTime<i16, Interpolation::Linear>(time, atChannel, source.Samples, source.SampleCount, source.SampleRate, source.ChannelCount));
			pcmCount++;
		}

		const i32 averagePCM = (pcmSum / pcmCount);
		const f32 normalizedPCM = averagePCM / static_cast<f32>(std::numeric_limits<i16>::max());

		return normalizedPCM;
	}
}
