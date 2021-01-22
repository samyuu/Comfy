#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ISampleProvider.h"

namespace Comfy::Audio
{
	class MemorySampleProvider : public ISampleProvider, NonCopyable
	{
		friend class DecoderFactory;

	public:
		MemorySampleProvider();
		~MemorySampleProvider();

		i64 ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead) override;
		i64 GetFrameCount() const override;

		u32 GetChannelCount() const override;
		u32 GetSampleRate() const override;
		const i16* GetRawSampleView() const override;

	private:
		u32 channelCount;
		u32 sampleRate;

		size_t sampleCount;
		std::unique_ptr<i16[]> sampleData;
	};
}
