#pragma once
#include "SampleProvider/MemorySampleProvider.h"
#include "Core/TimeSpan.h"

namespace Comfy::Audio
{
	class Waveform
	{
	public:
		Waveform();
		~Waveform();

		void Calculate(MemorySampleProvider* audioStream, TimeSpan timePerPixel);
		float GetPcmForPixel(int64_t pixel) const;
		size_t GetPixelCount() const;

	protected:
		// NOTE: Mapping of pixel <-> averaged PCM
		std::vector<float> pixelPCMs;
	};
}
