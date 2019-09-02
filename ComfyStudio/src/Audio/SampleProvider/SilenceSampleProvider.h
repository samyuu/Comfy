#pragma once
#include "ISampleProvider.h"

namespace Audio
{
	class SilenceSampleProvider : public ISampleProvider
	{
	public:
		virtual int64_t ReadSamples(int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead, uint32_t channelsToFill) override;
		virtual int64_t GetFrameCount() const override;

		virtual uint32_t GetChannelCount() const override;
		virtual uint32_t GetSampleRate() const override;
	};
}