#include "MemorySampleProvider.h"
#include "Audio/Core/AudioEngine.h"
#include <assert.h>

namespace Audio
{
	MemorySampleProvider::MemorySampleProvider()
	{
	}

	MemorySampleProvider::~MemorySampleProvider()
	{
	}

	int64_t MemorySampleProvider::ReadSamples(int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead, uint32_t channelsToFill)
	{
		std::fill(bufferToFill, bufferToFill + (framesToRead * channelsToFill), 0);

		if ((frameOffset + framesToRead) <= 0 || frameOffset > GetFrameCount())
		{
			// fill the buffer with silence
			return framesToRead;
		}

		if (frameOffset < 0 && (frameOffset + framesToRead) > 0)
		{
			int64_t nonSilentSamples = (framesToRead + frameOffset) * channelsToFill;
			int16_t* nonSilentBuffer = (bufferToFill - (frameOffset * channelsToFill));

			// fill a portion of the buffer
			std::copy(sampleData.data(), sampleData.data() + nonSilentSamples, nonSilentBuffer);

			return framesToRead;
		}

		// fill the whole buffer
		int16_t* sampleSource = &sampleData[frameOffset * channelsToFill];
		int64_t framesRead = ((frameOffset + framesToRead) > GetFrameCount()) ? (GetFrameCount() - frameOffset) : framesToRead;
		std::copy(sampleSource, sampleSource + (framesRead * channelsToFill), bufferToFill);

		return framesToRead;
	}

	int64_t MemorySampleProvider::GetFrameCount() const
	{
		return static_cast<int64_t>(sampleData.size()) / channelCount;
	}

	uint32_t MemorySampleProvider::GetChannelCount() const
	{
		return channelCount;
	}

	uint32_t MemorySampleProvider::GetSampleRate() const
	{
		return sampleRate;
	}
}