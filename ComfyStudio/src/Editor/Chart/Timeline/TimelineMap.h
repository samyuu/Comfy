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
		TimeSpan GetTimeAt(BeatTick tick) const;
		TimeSpan GetLastCalculatedTime() const;

		BeatTick GetTickAt(TimeSpan time) const;
		BeatTick GetTickAtFixedTempo(TimeSpan time, Tempo tempo) const;

		void CalculateMapTimes(SortedTempoMap& tempoMap);

	private:
		// NOTE: Pre calculated tick times up to the last tempo change
		std::vector<TimeSpan> tickTimes;
		Tempo firstTempo, lastTempo;
	};
}
