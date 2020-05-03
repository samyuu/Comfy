#pragma once
#include "Types.h"

namespace Comfy::Audio
{
	class ISampleProvider
	{
	public:
		virtual i64 ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead, u32 channelsToFill) = 0;
		virtual i64 GetFrameCount() const = 0;

		virtual u32 GetChannelCount() const = 0;
		virtual u32 GetSampleRate() const = 0;
	};
}
