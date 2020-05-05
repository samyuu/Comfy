#include "MemorySampleProvider.h"
#include "Audio/Core/AudioEngine.h"

namespace Comfy::Audio
{
	MemorySampleProvider::MemorySampleProvider()
	{
	}

	MemorySampleProvider::~MemorySampleProvider()
	{
	}

	i64 MemorySampleProvider::ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead, u32 channelsToFill)
	{
		std::fill(bufferToFill, bufferToFill + (framesToRead * channelsToFill), 0);

		if ((frameOffset + framesToRead) <= 0 || frameOffset > GetFrameCount())
		{
			// NOTE: Fill the buffer with silence
			return framesToRead;
		}

		if (frameOffset < 0 && (frameOffset + framesToRead) > 0)
		{
			const i64 nonSilentSamples = (framesToRead + frameOffset) * channelsToFill;
			i16* nonSilentBuffer = (bufferToFill - (frameOffset * channelsToFill));

			// NOTE: Fill a portion of the buffer
			std::copy(sampleData.get(), sampleData.get() + nonSilentSamples, nonSilentBuffer);

			return framesToRead;
		}

		// NOTE: Fill the whole buffer
		const i16* sampleSource = &sampleData[frameOffset * channelsToFill];
		const i64 framesRead = ((frameOffset + framesToRead) > GetFrameCount()) ? (GetFrameCount() - frameOffset) : framesToRead;
		
		std::copy(sampleSource, sampleSource + (framesRead * channelsToFill), bufferToFill);
		return framesToRead;
	}

	i64 MemorySampleProvider::GetFrameCount() const
	{
		return static_cast<i64>(sampleCount) / channelCount;
	}

	u32 MemorySampleProvider::GetChannelCount() const
	{
		return channelCount;
	}

	u32 MemorySampleProvider::GetSampleRate() const
	{
		return sampleRate;
	}

	const i16* MemorySampleProvider::GetRawSampleView() const
	{
		return sampleData.get();
	}
}
