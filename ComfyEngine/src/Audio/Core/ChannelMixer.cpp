#include "ChannelMixer.h"
#include "SampleMix.h"
#include <assert.h>

namespace Comfy::Audio
{
	i64 ChannelMixer::MixChannels(ISampleProvider& sampleProvider, i16 bufferToFill[], i64 frameOffset, i64 framesToRead)
	{
		const auto sourceChannels = sampleProvider.GetChannelCount();

		const u64 samplesToRead = framesToRead * sourceChannels;
		if (sampleSwapBuffer.size() < samplesToRead)
			sampleSwapBuffer.resize(samplesToRead);

		const i64 framesRead = sampleProvider.ReadSamples(sampleSwapBuffer.data(), frameOffset, framesToRead, sourceChannels);
		const i64 samplesRead = framesRead * sourceChannels;

		if (sourceChannels < targetChannels)
		{
			// NOTE: Duplicate existing channel(s)
			size_t targetIndex = 0;
			for (i64 i = 0; i < samplesRead; i++)
			{
				for (size_t c = 0; c < targetChannels; c++)
					bufferToFill[targetIndex++] = sampleSwapBuffer[i];
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
			case MixingBehavior::Ignore:
			{
				// NOTE: Remove extra channel(s)
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = sampleSwapBuffer[swapBufferIndex + 0];
					bufferToFill[i++] = sampleSwapBuffer[swapBufferIndex + 1];
					swapBufferIndex += sourceChannels;
				}

				break;
			}
			case MixingBehavior::Combine:
			{
				// NOTE: Mix extra channel(s)
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = MixSamples(sampleSwapBuffer[swapBufferIndex + 0], sampleSwapBuffer[swapBufferIndex + 2]);
					bufferToFill[i++] = MixSamples(sampleSwapBuffer[swapBufferIndex + 1], sampleSwapBuffer[swapBufferIndex + 3]);
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
