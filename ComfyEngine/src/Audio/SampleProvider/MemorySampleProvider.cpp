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

	i64 MemorySampleProvider::ReadSamples(i16 bufferToFill[], i64 frameOffset, i64 framesToRead)
	{
		std::fill(bufferToFill, bufferToFill + (framesToRead * channelCount), 0);

		const i64 sourceFrameCount = GetFrameCount();

		if ((frameOffset + framesToRead) <= 0 || frameOffset > sourceFrameCount)
			return framesToRead;

		if (frameOffset < 0 && (frameOffset + framesToRead) > 0)
		{
			const i64 nonSilentSamples = (framesToRead + frameOffset) * channelCount;
			i16* nonSilentBuffer = (bufferToFill - (frameOffset * channelCount));

			std::copy(
				sampleData.get(),
				Min<i16*>(sampleData.get() + nonSilentSamples, sampleData.get() + sampleCount),
				Min<i16*>(nonSilentBuffer, bufferToFill + (framesToRead * channelCount)));
		}
		else
		{
			const i16* sampleSource = &sampleData[frameOffset * channelCount];
			const i64 framesRead = ((framesToRead + frameOffset) > sourceFrameCount) ? (sourceFrameCount - frameOffset) : framesToRead;

			std::copy(sampleSource, sampleSource + (framesRead * channelCount), bufferToFill);
		}

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
