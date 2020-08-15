#pragma once
#include "SortedTargetList.h"
#include "SortedTempoMap.h"
#include "Timeline/TimelineMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	struct Chart
	{
	public:
		// NOTE: In case there is no audio file to take as a reference
		static constexpr TimeSpan FallbackDuration = TimeSpan::FromMinutes(1.0);

	public:
		SortedTargetList Targets;
		SortedTempoMap TempoMap;
		TimelineMap TimelineMap;

		TimeSpan StartOffset = TimeSpan::Zero();
		TimeSpan Duration = FallbackDuration;
	};
}
