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

		// TODO: Different methods to set source provider and change timePerPixel scale
		void Calculate(ISampleProvider& sampleProvider, TimeSpan timePerPixel);
		void Clear();

		// NOTE: Performs *no* bounds checking
		float GetPCMForPixel(i64 pixel);
		size_t GetPixelCount() const;

	private:
		// TODO: Implement implement shared ownership to avoid copying all samples
		ISampleProvider* lastSampleProvider = nullptr;

		size_t sampleCount = 0;
		u32 channelCount = 0;
		f64 sampleRate = 0.0;
		f64 secondsPerPixel = 0.0;
		
		// NOTE: Copy to avoid any ownership issues with ISampleProvider becoming unavailable
		std::unique_ptr<i16[]> sampleDataCopy = nullptr;

		size_t pixelCount = 0;
		std::vector<bool> cachedPixelBits;
		std::unique_ptr<f32[]> cachedPixelPCMs = nullptr;
	};
}
