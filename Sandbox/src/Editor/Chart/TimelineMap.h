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

		TimelineTick TimelineLength();
		TimeSpan GetTimeAt(TimelineTick tick);
		TimelineTick GetTickAt(TimeSpan time);

		static TimelineMap CalculateMapTimes(TempoMap& tempoMap, size_t barCount);

	private:
		std::vector<TimeSpan> tickTimes;
	};
}