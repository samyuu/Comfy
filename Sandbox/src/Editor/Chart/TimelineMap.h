#pragma once
#include "../../TimeSpan.h"
#include "TempoMap.h"

namespace Editor
{
	class TimelineMap
	{
	public:
		TimelineMap();
		TimelineMap(std::vector<TimeSpan>& times);

		TimeSpan GetTimeAt(TimelineTick tick);
		TimelineTick GetTickAt(TimeSpan time);

		static TimelineMap CalculateMapTimes(TempoMap& tempoMap);

	private:
		// pre calculated tick times up to the last tempo change
		std::vector<TimeSpan> tickTimes;
	};
}