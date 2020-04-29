#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Audio/SampleProvider/ISampleProvider.h"

namespace Comfy::Audio
{
	class ChannelMixer
	{
	public:
		enum class MixingBehavior
		{
			Ignore, Mix, Count
		};

	public:
		i64 MixChannels(ISampleProvider* sampleProvider, i16 bufferToFill[], i64 frameOffset, i64 framesToRead);

		MixingBehavior GetMixingBehavior() const;
		void SetMixingBehavior(MixingBehavior value);

		u32 GetSourceChannels() const;
		void SetSourceChannels(u32 value);
		
		u32 GetTargetChannels() const;
		void SetTargetChannels(u32 value);

	protected:
		MixingBehavior mixingBehavior = MixingBehavior::Ignore;
		std::vector<i16> sampleSwapBuffer;
		
		u32 sourceChannels = 0;
		u32 targetChannels = 0;
	};
}
