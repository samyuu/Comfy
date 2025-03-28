#pragma once
#include "Types.h"

namespace Comfy::Audio
{
	class ISampleProvider
	{
	public:
		virtual ~ISampleProvider() = default;

		virtual i64 ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead) = 0;
		virtual i64 GetFrameCount() const = 0;

		virtual u32 GetChannelCount() const = 0;
		virtual u32 GetSampleRate() const = 0;

		// NOTE: Optional raw view for optimization purposes
		virtual const i16* GetRawSampleView() const = 0;
	};
}
