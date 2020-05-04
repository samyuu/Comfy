#pragma once
#include "Types.h"
#include "Audio/SampleProvider/MemorySampleProvider.h"
#include "Time/TimeSpan.h"

namespace Comfy::Audio
{
	class Waveform
	{
	public:
		Waveform();
		~Waveform();

		void Calculate(MemorySampleProvider* audioStream, TimeSpan timePerPixel);
		float GetPcmForPixel(i64 pixel) const;
		size_t GetPixelCount() const;

	protected:
		// NOTE: Mapping of pixel <-> averaged PCM
		std::vector<float> pixelPCMs;
	};
}
