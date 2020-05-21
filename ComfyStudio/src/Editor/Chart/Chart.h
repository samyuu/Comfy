#pragma once
#include "TargetList.h"
#include "TempoMap.h"
#include "Timeline/TimelineMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	class Chart
	{
	public:
		TargetList& GetTargets();
		TempoMap& GetTempoMap();
		TimelineMap& GetTimelineMap();

		TimeSpan GetStartOffset() const;
		void SetStartOffset(TimeSpan value);

		TimeSpan GetDuration() const;
		void SetDuration(TimeSpan value);

	private:
		TargetList targets;
		TempoMap tempoMap;
		TimelineMap timelineMap;

		TimeSpan startOffset = TimeSpan::FromSeconds(0.0);
		TimeSpan duration = TimeSpan::FromMinutes(1.0);
	};
}
