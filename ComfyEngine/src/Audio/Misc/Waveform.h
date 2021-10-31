#pragma once
#include "Types.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Time/TimeSpan.h"

namespace Comfy::Audio
{
	class Waveform
	{
	public:
		Waveform() = default;
		~Waveform() = default;

	public:
		void SetSource(std::shared_ptr<ISampleProvider> sampleProvider);

		// NOTE: Invalidates previously cached results, should therefore not be called every frame
		void SetScale(TimeSpan timePerPixel);
		TimeSpan GetTimePerPixel() const;
		
		void Clear();

		float GetNormalizedPCMForPixel(i64 pixel, u32 channelIndex);

		size_t GetPixelCount() const;
		u32 GetChannelCount() const;

	private:
		float AveragePCMAtPixel(double atPixel, u32 atChannel) const;

		struct SourceData
		{
			std::shared_ptr<ISampleProvider> SampleProvider;
			const i16* SamplesView;
			size_t SampleCount;
			u32 ChannelCount;
			f64 SampleRate;
		} source = {};

		f64 secondsPerPixel = 0.0;
		f64 secondsPerSample = 0.0;

		size_t perChannelPixelCount = 0;
		int channelIndex = -1;

		template <typename T>
		struct FixedPointPCM
		{
			static_assert(std::is_integral_v<T>);
			T Value;

			void operator=(f32 newValue) { Value = static_cast<T>(newValue * static_cast<f32>(std::numeric_limits<T>::max())); }
			operator f32() const { return static_cast<f32>(Value) * (1.0f / static_cast<f32>(std::numeric_limits<T>::max())); }
		};

		// NOTE: The precision loss at the scales these are used at typically equate to less than a pixel.
		//		 Unsigned integers can be used in this case beacuse only absolute values are measured though that might not be the case in the future
		using CachedPCMType = FixedPointPCM<u8>;

		std::vector<bool> cachedPixelBits;
		std::unique_ptr<CachedPCMType[]> cachedPixelPCMs = nullptr;
	};
}
