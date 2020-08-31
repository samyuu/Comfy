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
			source.SamplesView = sampleProvider->GetRawSampleView();
			source.SampleCount = sampleProvider->GetFrameCount() * sampleProvider->GetChannelCount();
			source.ChannelCount = sampleProvider->GetChannelCount();
			source.SampleRate = static_cast<f64>(sampleProvider->GetSampleRate());
		}
	}

	void Waveform::SetScale(TimeSpan timePerPixel)
	{
		if (source.SampleProvider == nullptr)
		{
			perChannelPixelCount = 0;
			return;
		}

		secondsPerPixel = timePerPixel.TotalSeconds();
		secondsPerSample = 1.0 / (source.SampleRate);

		const auto samplesPerPixel = static_cast<f64>(secondsPerPixel * source.SampleRate * source.ChannelCount);
		const auto newPerChannelPixelCount = static_cast<size_t>(source.SampleCount / samplesPerPixel);
		const auto newTotalPixelCount = newPerChannelPixelCount * source.ChannelCount;

		if (cachedPixelPCMs == nullptr || newTotalPixelCount > cachedPixelBits.size())
		{
			cachedPixelPCMs = std::make_unique<CachedPCMType[]>(newTotalPixelCount);
			cachedPixelBits.resize(newTotalPixelCount);
		}

		std::fill(cachedPixelBits.begin(), cachedPixelBits.begin() + newTotalPixelCount, 0);
		perChannelPixelCount = newPerChannelPixelCount;
	}

	TimeSpan Waveform::GetTimePerPixel() const
	{
		return TimeSpan::FromSeconds(secondsPerPixel);
	}

	void Waveform::Clear()
	{
		source = {};
		secondsPerPixel = 0.0;
		secondsPerSample = 0.0;
		perChannelPixelCount = 0;
		cachedPixelBits.clear();
		cachedPixelBits.shrink_to_fit();
		cachedPixelPCMs = nullptr;
	}

	float Waveform::GetNormalizedPCMForPixel(i64 pixel, u32 channelIndex)
	{
		const auto pixelIndex = static_cast<size_t>((pixel * source.ChannelCount) + channelIndex);
		if (!InBounds(pixelIndex, cachedPixelBits) || channelIndex >= source.ChannelCount)
			return 0.0f;

		if (cachedPixelBits[pixelIndex])
			return cachedPixelPCMs[pixelIndex];

		const auto calculatedResult = AveragePCMAtPixel(static_cast<f64>(pixel), channelIndex);

		cachedPixelBits[pixelIndex] = true;
		cachedPixelPCMs[pixelIndex] = calculatedResult;

		// NOTE: Explicitly read the written value to force a type conversion
		return cachedPixelPCMs[pixelIndex];
	}

	size_t Waveform::GetPixelCount() const
	{
		return perChannelPixelCount;
	}

	u32 Waveform::GetChannelCount() const
	{
		return source.ChannelCount;
	}

	float Waveform::AveragePCMAtPixel(double atPixel, u32 atChannel) const
	{
		assert(source.SampleProvider != nullptr);

		const f64 pixelStartTime = (atPixel * secondsPerPixel);
		const f64 pixelEndTime = ((atPixel + 1.0) * secondsPerPixel);

		i32 pcmSum = 0, pcmCount = 0;
		for (f64 time = pixelStartTime; time < pixelEndTime; time += secondsPerSample)
		{
			pcmSum += glm::abs(SampleAtTime<i16, Interpolation::Linear>(time, atChannel, source.SamplesView, source.SampleCount, source.SampleRate, source.ChannelCount));
			pcmCount++;
		}

		const i32 averagePCM = (pcmSum / pcmCount);
		const f32 normalizedPCM = averagePCM / static_cast<f32>(std::numeric_limits<i16>::max());

		return normalizedPCM;
	}
}
