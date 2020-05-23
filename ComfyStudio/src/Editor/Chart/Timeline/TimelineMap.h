#pragma once
#include "Editor/Chart/TempoMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	class TimelineMap
	{
	public:
		TimelineMap() = default;
		TimelineMap(std::vector<TimeSpan>& times, Tempo firstTempo, Tempo lastTempo);

	public:
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
