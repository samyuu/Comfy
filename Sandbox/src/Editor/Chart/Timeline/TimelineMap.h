#pragma once
#include "Editor/Chart/TempoMap.h"
#include "Core/TimeSpan.h"

namespace Editor
{
	class TimelineMap
	{
	public:
		TimelineMap();
		TimelineMap(Vector<TimeSpan>& times, Tempo firstTempo, Tempo lastTempo);

		TimeSpan GetTimeAt(TimelineTick tick) const;
		TimeSpan GetLastCalculatedTime() const;
		TimelineTick GetTickAt(TimeSpan time) const;

		void CalculateMapTimes(TempoMap& tempoMap);

	private:
		// pre calculated tick times up to the last tempo change
		Vector<TimeSpan> tickTimes;
		Tempo firstTempo, lastTempo;
	};
}