#include "ChannelMixer.h"
#include "SampleMix.h"

namespace Comfy::Audio
{
	i64 ChannelMixer::MixChannels(ISampleProvider& sampleProvider, i16 bufferToFill[], i64 frameOffset, i64 framesToRead)
	{
		const auto sourceChannels = sampleProvider.GetChannelCount();
		const auto mixBuffer = GetMixSampleBuffer(framesToRead * sourceChannels);

		const i64 framesRead = sampleProvider.ReadSamples(mixBuffer, frameOffset, framesToRead);
		return MixChannels(sourceChannels, mixBuffer, framesRead, bufferToFill, frameOffset, framesToRead);
	}

	i64 ChannelMixer::MixChannels(u32 sourceChannels, i16 mixBuffer[], i64 framesRead, i16 bufferToFill[], i64 frameOffset, i64 framesToRead)
	{
		const i64 samplesRead = framesRead * sourceChannels;

		if (sourceChannels < targetChannels)
		{
			// NOTE: Duplicate existing channel(s)
			size_t targetIndex = 0;
			for (i64 i = 0; i < samplesRead; i++)
			{
				for (size_t c = 0; c < targetChannels; c++)
					bufferToFill[targetIndex++] = mixBuffer[i];
			}
		}
		else
		{
			if (sourceChannels < 4 || targetChannels != 2)
			{
				// TODO: Implement a more generic solution (?)
				std::fill(bufferToFill, bufferToFill + (framesToRead * targetChannels), 0);
				return framesToRead;
			}

			int swapBufferIndex = 0;

			switch (mixingBehavior)
			{
			case MixingBehavior::IgnoreTrailing:
			{
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = mixBuffer[swapBufferIndex + 0];
					bufferToFill[i++] = mixBuffer[swapBufferIndex + 1];
					swapBufferIndex += sourceChannels;
				}
				break;
			}
			case MixingBehavior::IgnoreLeading:
			{
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = mixBuffer[swapBufferIndex + 2];
					bufferToFill[i++] = mixBuffer[swapBufferIndex + 3];
					swapBufferIndex += sourceChannels;
				}
				break;
			}
			case MixingBehavior::Combine:
			{
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = MixSamples(mixBuffer[swapBufferIndex + 0], mixBuffer[swapBufferIndex + 2]);
					bufferToFill[i++] = MixSamples(mixBuffer[swapBufferIndex + 1], mixBuffer[swapBufferIndex + 3]);
					swapBufferIndex += sourceChannels;
				}
				break;
			}

			default:
				return 0;
			}
		}

		return framesRead;
	}

	i16* ChannelMixer::GetMixSampleBuffer(size_t sampleCount)
	{
		if (sampleMixBuffer.size() < sampleCount)
			sampleMixBuffer.resize(sampleCount);

		return sampleMixBuffer.data();
	}

	ChannelMixer::MixingBehavior ChannelMixer::GetMixingBehavior() const
	{
		return mixingBehavior;
	}

	void ChannelMixer::SetMixingBehavior(ChannelMixer::MixingBehavior value)
	{
		mixingBehavior = value;
	}

	u32 ChannelMixer::GetTargetChannels() const
	{
		return targetChannels;
	}

	void ChannelMixer::SetTargetChannels(u32 value)
	{
		targetChannels = value;
	}
}
