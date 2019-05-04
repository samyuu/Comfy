#include "ChannelMixer.h"
#include <assert.h>>

ChannelMixer::ChannelMixer(uint32_t channelTargetCount) : channelTargetCount(channelTargetCount)
{

}

void ChannelMixer::MixChannels(int16_t** sampleDataPtr, size_t* sampleCountPtr, uint32_t* channelCountPtr)
{
	assert(*sampleDataPtr);
	assert(*channelCountPtr != channelTargetCount);

	uint32_t channelCount = *channelCountPtr;
	uint32_t sampleCount = *sampleCountPtr;
	uint32_t newSampleCount = (sampleCount / channelCount) * channelTargetCount;

	int16_t* mixedSampleData = new int16_t[newSampleCount];
	{
		if (*channelCountPtr < channelTargetCount)
		{
			// Duplicate existing channel(s)
			// -----------------------------

			size_t targetIndex = 0;
			for (size_t i = 0; i < sampleCount; i++)
			{
				for (size_t c = 0; c < channelTargetCount; c++)
					mixedSampleData[targetIndex++] = (*sampleDataPtr)[i];
			}
		}
		else
		{
			// Remove extra channels
			// ---------------------

			// set to 2 to use the diva vocal track for example
			// might also want to combine different tracks instead of removing them
			constexpr int channelOffset = 0;
			
			int sampleIndex = channelOffset;
			for (size_t i = 0; i < newSampleCount;)
			{
				for (size_t c = 0; c < channelTargetCount; c++)
					mixedSampleData[i++] = (*sampleDataPtr)[sampleIndex + c];

				sampleIndex += channelCount;
			}
		}
	}

	*sampleCountPtr = newSampleCount;
	*channelCountPtr = channelTargetCount;

	delete[] * sampleDataPtr;
	*sampleDataPtr = mixedSampleData;
}