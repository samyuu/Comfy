#pragma once
#include "Types.h"

namespace Comfy::Audio
{
	class ISampleProvider
	{
	public:
		virtual int64_t ReadSamples(int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead, uint32_t channelsToFill) = 0;
		virtual int64_t GetFrameCount() const = 0;

		virtual uint32_t GetChannelCount() const = 0;
		virtual uint32_t GetSampleRate() const = 0;
	};
}
