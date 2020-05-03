#include "ChannelMixer.h"
#include "SampleMixer.h"
#include <assert.h>

namespace Comfy::Audio
{
	i64 ChannelMixer::MixChannels(ISampleProvider* sampleProvider, i16 bufferToFill[], i64 frameOffset, i64 framesToRead)
	{
		u64 samplesToRead = framesToRead * sourceChannels;
		if (sampleSwapBuffer.size() < samplesToRead)
			sampleSwapBuffer.resize(samplesToRead);

		i64 framesRead = sampleProvider->ReadSamples(sampleSwapBuffer.data(), frameOffset, framesToRead, sourceChannels);
		i64 samplesRead = framesRead * sourceChannels;

		if (sourceChannels < targetChannels)
		{
			// Duplicate existing channel(s)
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
				// TODO: implement a more generic solution (?)
				std::fill(bufferToFill, bufferToFill + (framesToRead * targetChannels), 0);
				return framesToRead;
			}

			int swapBufferIndex = 0;

			switch (mixingBehavior)
			{
			case MixingBehavior::Ignore:
			{
				// Remove extra channel(s)
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = sampleSwapBuffer[swapBufferIndex + 0];
					bufferToFill[i++] = sampleSwapBuffer[swapBufferIndex + 1];
					swapBufferIndex += sourceChannels;
				}

				break;
			}
			case MixingBehavior::Mix:
			{
				// Mix extra channel(s)
				for (i64 i = 0; i < framesRead * targetChannels;)
				{
					bufferToFill[i++] = SampleMixer::MixSamples(sampleSwapBuffer[swapBufferIndex + 0], sampleSwapBuffer[swapBufferIndex + 2]);
					bufferToFill[i++] = SampleMixer::MixSamples(sampleSwapBuffer[swapBufferIndex + 1], sampleSwapBuffer[swapBufferIndex + 3]);
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

	u32 ChannelMixer::GetSourceChannels() const
	{
		return sourceChannels;
	}

	void ChannelMixer::SetSourceChannels(u32 value)
	{
		sourceChannels = value;
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
