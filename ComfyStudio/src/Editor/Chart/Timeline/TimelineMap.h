#pragma once
#include "Editor/Chart/SortedTempoMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	class TimelineMap
	{
	public:
		TimelineMap() = default;
		~TimelineMap() = default;

	public:
		TimeSpan GetTimeAt(TimelineTick tick) const;
		TimeSpan GetLastCalculatedTime() const;

		TimelineTick GetTickAt(TimeSpan time) const;
		TimelineTick GetTickAtFixedTempo(TimeSpan time, Tempo tempo) const;

		void CalculateMapTimes(SortedTempoMap& tempoMap);

	private:
		// NOTE: Pre calculated tick times up to the last tempo change
		std::vector<TimeSpan> tickTimes;
		Tempo firstTempo, lastTempo;
	};
}
