#include "Chart.h"

namespace Editor
{
	TargetList& Chart::GetTargets()
	{
		return targets;
	}
	
	TempoMap& Chart::GetTempoMap()
	{
		return tempoMap;
	}

	TimelineMap& Chart::GetTimelineMap()
	{
		return timelineMap;
	}
	
	TimeSpan Chart::GetStartOffset() const
	{
		return startOffset;
	}
	
	void Chart::SetStartOffset(TimeSpan value)
	{
		startOffset = value;
	}
	
	TimeSpan Chart::GetDuration() const
	{
		return duration;
	}

	void Chart::SetDuration(TimeSpan value)
	{
		duration = value;
	}
}