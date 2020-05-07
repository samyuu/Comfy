#pragma once
#include "Types.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Time/TimeSpan.h"

namespace Comfy::Audio
{
	// TODO: Optimally this should calculate a cached pixel PCM representation async and while processing return dynamic results instead
	class Waveform
	{
	public:
		Waveform() = default;
		~Waveform() = default;

		void Calculate(ISampleProvider& sampleProvider, TimeSpan timePerPixel);
		void Clear();

		float GetPcmForPixel(i64 pixel) const;
		size_t GetPixelCount() const;

	protected:
		// NOTE: Mapping of pixel <-> averaged PCM
		std::unique_ptr<float[]> pixelPCMs = nullptr;
		i64 pixelCount = 0;
	};
}
