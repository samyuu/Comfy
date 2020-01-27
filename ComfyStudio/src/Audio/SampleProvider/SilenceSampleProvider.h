#pragma once
#include "ISampleProvider.h"

namespace Audio
{
	class SilenceSampleProvider : public ISampleProvider
	{
	public:
		int64_t ReadSamples(int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead, uint32_t channelsToFill) override;
		int64_t GetFrameCount() const override;

		uint32_t GetChannelCount() const override;
		uint32_t GetSampleRate() const override;
	};
}