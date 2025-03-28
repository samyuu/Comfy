#pragma once
#include "Types.h"
#include "ISampleProvider.h"

namespace Comfy::Audio
{
	class SilenceSampleProvider : public ISampleProvider
	{
	public:
		i64 ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead) override;
		i64 GetFrameCount() const override;

		u32 GetChannelCount() const override;
		u32 GetSampleRate() const override;
		const i16* GetRawSampleView() const override;
	};
}
