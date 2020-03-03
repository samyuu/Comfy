#pragma once
#include "TargetList.h"
#include "TempoMap.h"
#include "Timeline/TimelineMap.h"
#include "Core/TimeSpan.h"

namespace Comfy::Editor
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

		TimeSpan startOffset = 0.0;
		TimeSpan duration = TimeSpan::FromMinutes(1.0);
	};
}
