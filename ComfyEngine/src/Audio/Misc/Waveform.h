#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Time/TimeSpan.h"

namespace Comfy::Audio
{
	class Waveform
	{
	public:
		Waveform() = default;
		~Waveform() = default;

		void SetSource(std::shared_ptr<ISampleProvider> sampleProvider);
		void SetScale(TimeSpan timePerPixel);
		void Clear();

		float GetNormalizedPCMForPixel(i64 pixel);
		size_t GetPixelCount() const;

	private:
		float AveragePCMAtPixel(double atPixel, u32 atChannel) const;

		struct SourceData
		{
			std::shared_ptr<ISampleProvider> SampleProvider;
			const i16* Samples;
			size_t SampleCount;
			u32 ChannelCount;
			f64 SampleRate;
		} source = {};

		f64 secondsPerPixel = 0.0;
		f64 secondsPerSample = 0.0;

		size_t pixelCount = 0;

		std::vector<bool> cachedPixelBits;
		std::unique_ptr<f32[]> cachedPixelPCMs = nullptr;
	};
}
