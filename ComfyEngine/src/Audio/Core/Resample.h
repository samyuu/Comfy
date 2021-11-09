#pragma once
#include "Types.h"

namespace Comfy::Audio
{
	namespace Interpolation
	{
		struct NearestNeighbor
		{
			static constexpr f64 Interpolate(f64 startValue, f64 endValue, f64 inbetween)
			{
				return (inbetween <= 0.5) ? startValue : endValue;
			}
		};

		struct Linear
		{
			static constexpr f64 Interpolate(f64 startValue, f64 endValue, f64 inbetween)
			{
				return ((1.0 - inbetween) * startValue) + (inbetween * endValue);
			}
		};

		struct Cosine
		{
			static f64 Interpolate(f64 startValue, f64 endValue, f64 inbetween)
			{
				const auto temp = (1.0 - std::cos(inbetween * glm::pi<f64>())) * 0.5;
				return (startValue * (1.0 - temp) + endValue * temp);
			}
		};

		using Default = Linear;
	}

	template <typename SampleType>
	constexpr SampleType SampleAtFrame(i64 atFrame, u32 atChannel, const SampleType* sampels, size_t sampleCount, u32 channelCount)
	{
		const auto sampleIndex = static_cast<size_t>((atFrame * channelCount) + atChannel);
		return sampleIndex < sampleCount ? sampels[sampleIndex] : static_cast<SampleType>(0);
	}

	template <typename SampleType, typename InterpolationType>
	constexpr SampleType SampleAtTime(f64 atSecond, u32 atChannel, const SampleType* samples, size_t sampleCount, f64 sampleRate, u32 channelCount)
	{
		const f64 frameFaction = (atSecond * sampleRate);

		const i64 startFrame = static_cast<i64>(frameFaction);
		const i64 endFrame = (startFrame + 1);

		const auto startValue = SampleAtFrame<SampleType>(startFrame, atChannel, samples, sampleCount, channelCount);
		const auto endValue = SampleAtFrame<SampleType>(endFrame, atChannel, samples, sampleCount, channelCount);
		const auto inbetween = (frameFaction - static_cast<f64>(startFrame));

		constexpr auto maxFloatSampleValue = static_cast<f64>(std::numeric_limits<SampleType>::max());
		const auto normalizedStart = static_cast<f64>(startValue / maxFloatSampleValue);
		const auto normalizedEnd = static_cast<f64>(endValue / maxFloatSampleValue);

		const auto normalizedResult = InterpolationType::template Interpolate(normalizedStart, normalizedEnd, inbetween);
		const auto clampedResult = Clamp(normalizedResult, -1.0, 1.0);

		const auto sampleTypeResult = static_cast<SampleType>(clampedResult * maxFloatSampleValue);
		return sampleTypeResult;
	}

	// NOTE: Relatively low quallity resampling algorithm lacking a low pass filter, should however still be better than having sped up audio.
	//		 For any serious work it is strongly recommended to resample the audio externally
	template <typename SampleType, typename InterpolationType = Interpolation::Default>
	void Resample(std::unique_ptr<SampleType[]>& inOutSamples, size_t& inOutSampleCount, u32& inOutSampleRate, const u32 targetSampleRate, const u32 channelCount)
	{
		const auto inSamples = inOutSamples.get();
		const auto inSampleCount = inOutSampleCount;

		const auto sourceRate = static_cast<f64>(inOutSampleRate);
		const auto targetRate = static_cast<f64>(targetSampleRate);

		const auto inFrameCount = (inSampleCount / channelCount);
		const auto outFrameCount = static_cast<size_t>(inFrameCount * targetRate / sourceRate + 0.5);

		const auto outSampleCount = (outFrameCount * channelCount);
		auto outSamples = std::make_unique<SampleType[]>(outSampleCount);

		const f64 outFrameToSecond = (1.0 / targetRate);
		SampleType* outSampleWriteHead = outSamples.get();

		for (size_t frame = 0; frame < outFrameCount; frame++)
		{
			const f64 second = (static_cast<f64>(frame) * outFrameToSecond);

			for (u32 channel = 0; channel < channelCount; channel++)
				outSampleWriteHead[channel] = SampleAtTime<SampleType, InterpolationType>(second, channel, inSamples, inSampleCount, sourceRate, channelCount);

			outSampleWriteHead += channelCount;
		}

		inOutSamples = std::move(outSamples);
		inOutSampleCount = outSampleCount;
		inOutSampleRate = targetSampleRate;
	}
}
