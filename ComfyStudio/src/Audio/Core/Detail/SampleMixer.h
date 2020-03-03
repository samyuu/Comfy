#pragma once
#include "Types.h"

namespace Comfy::Audio
{
	class SampleMixer
	{
		SampleMixer() = delete;

	public:
		template <typename TSampleBase, typename TSampleTemp>
		static inline TSampleBase MixSamples(TSampleBase sampleA, TSampleBase sampleB)
		{
			const TSampleTemp result = static_cast<TSampleTemp>(sampleA) + static_cast<TSampleTemp>(sampleB);
			typedef std::numeric_limits<TSampleBase> SampleTypeRange;

			if (SampleTypeRange::max() < result)
				return SampleTypeRange::max();
			else if (SampleTypeRange::min() > result)
				return SampleTypeRange::min();
			else
				return static_cast<TSampleBase>(result);
		}

		static inline int16_t MixSamples(int16_t sampleA, int16_t sampleB)
		{
			return MixSamples<int16_t, int32_t>(sampleA, sampleB);
		}
	};
}
