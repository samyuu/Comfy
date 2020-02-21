#pragma once
#include "CoreTypes.h"
#include "ISampleProvider.h"

namespace Audio
{
	class MemorySampleProvider : public ISampleProvider, NonCopyable
	{
		friend class AudioDecoderFactory;

	public:
		MemorySampleProvider();
		~MemorySampleProvider();

		int64_t ReadSamples(int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead, uint32_t channelsToFill) override;
		int64_t GetFrameCount() const override;

		uint32_t GetChannelCount() const override;
		uint32_t GetSampleRate() const override;

		inline int16_t* GetSampleData() { return sampleData.data(); };

	private:
		uint32_t channelCount;
		uint32_t sampleRate;

		std::vector<int16_t> sampleData;
	};
}