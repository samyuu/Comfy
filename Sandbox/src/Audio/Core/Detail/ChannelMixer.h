#pragma once
#include "Types.h"
#include "Audio/SampleProvider/ISampleProvider.h"
#include "Core/CoreTypes.h"

namespace Audio
{
	class ChannelMixer
	{
	public:
		enum class MixingBehavior
		{
			Ignore, Mix, Count
		};

	public:
		int64_t MixChannels(ISampleProvider* sampleProvider, int16_t bufferToFill[], int64_t frameOffset, int64_t framesToRead);

		MixingBehavior GetMixingBehavior() const;
		void SetMixingBehavior(MixingBehavior value);

		uint32_t GetSourceChannels() const;
		void SetSourceChannels(uint32_t value);
		
		uint32_t GetTargetChannels() const;
		void SetTargetChannels(uint32_t value);

	protected:
		MixingBehavior mixingBehavior = MixingBehavior::Ignore;
		Vector<int16_t> sampleSwapBuffer;
		
		uint32_t sourceChannels = 0;
		uint32_t targetChannels = 0;
	};
}