#pragma once
#include "ISampleProvider.h"
#include "Core/CoreTypes.h"

namespace Audio
{
	class MemorySampleProvider : public ISampleProvider
	{
		friend class AudioDecoderFactory;

	public:
		MemorySampleProvider();
		MemorySampleProvider(const MemorySampleProvider& other) = delete;
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