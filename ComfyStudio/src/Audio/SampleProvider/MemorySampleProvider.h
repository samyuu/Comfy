#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ISampleProvider.h"

namespace Comfy::Audio
{
	class MemorySampleProvider : public ISampleProvider, NonCopyable
	{
		friend class AudioDecoderFactory;

	public:
		MemorySampleProvider();
		~MemorySampleProvider();

		i64 ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead, u32 channelsToFill) override;
		i64 GetFrameCount() const override;

		u32 GetChannelCount() const override;
		u32 GetSampleRate() const override;

		inline i16* GetSampleData() { return sampleData.data(); }

	private:
		u32 channelCount;
		u32 sampleRate;

		std::vector<i16> sampleData;
	};
}
