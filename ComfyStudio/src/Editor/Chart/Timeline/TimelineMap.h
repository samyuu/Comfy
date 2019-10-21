#pragma once
#include "Editor/Chart/TempoMap.h"
#include "Core/TimeSpan.h"

namespace Editor
{
	class TimelineMap
	{
	public:
		TimelineMap();
		TimelineMap(std::vector<TimeSpan>& times, Tempo firstTempo, Tempo lastTempo);

		TimeSpan GetTimeAt(TimelineTick tick) const;
		TimeSpan GetLastCalculatedTime() const;
		TimelineTick GetTickAt(TimeSpan time) const;

		void CalculateMapTimes(TempoMap& tempoMap);

	private:
		// NOTE: Pre calculated tick times up to the last tempo change
		std::vector<TimeSpan> tickTimes;
		Tempo firstTempo, lastTempo;
	};
}