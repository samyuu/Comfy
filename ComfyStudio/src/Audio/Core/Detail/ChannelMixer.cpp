#include "ChannelMixer.h"
#include "SampleMixer.h"
#include <assert.h>

namespace Audio
{
	int64_t ChannelMixer::MixChannels(ISampleProvider* sampleProvider, int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead)
	{
		uint64_t samplesToRead = framesToRead * sourceChannels;
		if (sampleSwapBuffer.size() < samplesToRead)
			sampleSwapBuffer.resize(samplesToRead);

		int64_t framesRead = sampleProvider->ReadSamples(sampleSwapBuffer.data(), frameOffset, framesToRead, sourceChannels);
		int64_t samplesRead = framesRead * sourceChannels;

		if (sourceChannels < targetChannels)
		{
			// Duplicate existing channel(s)
			size_t targetIndex = 0;
			for (int64_t i = 0; i < samplesRead; i++)
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
				for (int64_t i = 0; i < framesRead * targetChannels;)
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
				for (int64_t i = 0; i < framesRead * targetChannels;)
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

	uint32_t ChannelMixer::GetSourceChannels() const
	{
		return sourceChannels;
	}

	void ChannelMixer::SetSourceChannels(uint32_t value)
	{
		sourceChannels = value;
	}

	uint32_t ChannelMixer::GetTargetChannels() const
	{
		return targetChannels;
	}

	void ChannelMixer::SetTargetChannels(uint32_t value)
	{
		targetChannels = value;
	}
}