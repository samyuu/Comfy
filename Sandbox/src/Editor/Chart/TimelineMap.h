#pragma once
#include "../../TimeSpan.h"
#include "TempoMap.h"

namespace Editor
{
	class TimelineMap
	{
	public:
		TimelineMap();
		TimelineMap(std::vector<TimeSpan>& times, Tempo firstTempo, Tempo lastTempo);

		TimeSpan GetTimeAt(TimelineTick tick);
		TimeSpan GetLastCalculatedTime();
		TimelineTick GetTickAt(TimeSpan time);

		void CalculateMapTimes(TempoMap& tempoMap);

	private:
		// pre calculated tick times up to the last tempo change
		std::vector<TimeSpan> tickTimes;
		Tempo firstTempo, lastTempo;
	};
}