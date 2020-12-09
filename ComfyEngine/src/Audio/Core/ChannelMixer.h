#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Audio/SampleProvider/ISampleProvider.h"

namespace Comfy::Audio
{
	class ChannelMixer : NonCopyable
	{
	public:
		enum class MixingBehavior
		{
			Combine,
			IgnoreTrailing,
			IgnoreLeading,
			Count
		};

		static constexpr std::array<const char*, EnumCount<MixingBehavior>()> MixingBehaviorNames =
		{
			"Combine",
			"Ignore Trailing",
			"Ignore Leading",
		};

	public:
		i64 MixChannels(ISampleProvider& sampleProvider, i16 bufferToFill[], i64 frameOffset, i64 framesToRead);
		i64 MixChannels(u32 sourceChannels, i16 sampleSwapBuffer[], i64 framesRead, i16 bufferToFill[], i64 frameOffset, i64 framesToRead);

		i16* GetMixSampleBuffer(size_t sampleCount);

		MixingBehavior GetMixingBehavior() const;
		void SetMixingBehavior(MixingBehavior value);

		u32 GetTargetChannels() const;
		void SetTargetChannels(u32 value);

	private:
		MixingBehavior mixingBehavior = MixingBehavior::Combine;
		std::vector<i16> sampleMixBuffer;

		u32 targetChannels = 0;
	};
}
