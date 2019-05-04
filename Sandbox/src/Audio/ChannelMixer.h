#pragma once
#include <stdint.h>

class ChannelMixer
{
public:
	ChannelMixer(uint32_t channelTargetCount);

	void MixChannels(int16_t** sampleDataPtr, size_t* sampleCountPtr, uint32_t* channelCountPtr);

protected:
	uint32_t channelTargetCount;
};