#pragma once
#include "Types.h"

namespace Comfy::Audio
{
	template <typename SampleType, typename CalcType>
	constexpr SampleType MixSamples(SampleType sampleA, SampleType sampleB)
	{
		const CalcType result = static_cast<CalcType>(sampleA) + static_cast<CalcType>(sampleB);
		using SampleTypeRange = std::numeric_limits<SampleType>;

		if (SampleTypeRange::max() < result)
			return SampleTypeRange::max();
		else if (SampleTypeRange::min() > result)
			return SampleTypeRange::min();
		else
			return static_cast<SampleType>(result);
	}

	constexpr i16 MixSamples(i16 sampleA, i16 sampleB)
	{
		return MixSamples<i16, i32>(sampleA, sampleB);
	}
}
