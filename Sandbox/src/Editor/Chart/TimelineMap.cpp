#include "TimelineMap.h"

namespace Editor
{
	TimelineMap::TimelineMap()
	{
	}

	TimelineMap::TimelineMap(std::vector<TimeSpan>& times) : tickTimes(times)
	{
	}

	TimelineTick TimelineMap::TimelineLength()
	{
		return TimelineTick(tickTimes.size());
	}

	TimeSpan TimelineMap::GetTimeAt(TimelineTick tick)
	{
		int tickTimeCount = tickTimes.size();

		if (tickTimeCount <= tick.TotalTicks())
		{
			// out of bounds
			tickTimes.at(tickTimeCount - 1);
		}
		else
		{
			return tickTimes.at(tick.TotalTicks());
		}
	}

	TimelineTick TimelineMap::GetTickAt(TimeSpan time)
	{
		assert(false);
		return 0;
	}

	TimelineMap TimelineMap::CalculateMapTimes(TempoMap& tempoMap, size_t barCount)
	{
		const size_t timeCount = barCount * TimelineTick::TICKS_PER_BAR;

		std::vector<TimeSpan> tickTimes(timeCount);
		{
			// the time of when the last tempo change ended. lets use use higher precision multiplication
			double tempoChangeEndTime = 0.0;

			const size_t tempoChanges = tempoMap.TempoChangeCount();
			for (size_t b = 0; b < tempoChanges; b++)
			{
				TempoChange& tempoChange = tempoMap.GetTempoChangeAt(b);

				const double beatDuration = (60.0 / tempoChange.Tempo.BeatsPerMinute);
				const double beatTickDuration = (beatDuration / TimelineTick::TICKS_PER_BEAT);

				bool onlyTempo = (tempoChanges == 1);
				bool lastTempo = (b == (tempoChanges - 1));

				const size_t timesStart = tempoChange.Tick.TotalTicks();
				const size_t timesCount = (onlyTempo || lastTempo) ? (tickTimes.size()) : (tempoMap.GetTempoChangeAt(b + 1).Tick.TotalTicks());

				for (size_t i = 0, t = timesStart; t < timesCount; t++)
					tickTimes[t] = (beatTickDuration * i++) + tempoChangeEndTime;

				tempoChangeEndTime = tickTimes[timesCount - 1].TotalSeconds() + beatTickDuration;
			}
		}

		TimelineMap timelineTimes(tickTimes);

		//for (size_t i = 0; i <= 12 * TimelineTick::TICKS_PER_BAR; i++)
		//	printf("[%d] %fms\n", i, timelineTimes.GetTimeAt(TimelineTick::FromTicks(i)).Milliseconds());

		//for (size_t i = 0; i <= 12; i++)
		//	printf("[%d] %fms\n", i, timelineTimes.GetTimeAt(TimelineTick::FromBars(i)).Milliseconds());

		//for (size_t i = 0; i <= 12 * 4; i++)
		//	printf("[%d] %fms\n", i, timelineTimes.GetTimeAt(TimelineTick::FromTicks(i * TimelineTick::TICKS_PER_BEAT)).Milliseconds());

		for (size_t i = 0; i <= 100 * 4; i++)
			printf("[%d] %s\n", i, timelineTimes.GetTimeAt(TimelineTick::FromTicks(i * TimelineTick::TICKS_PER_BEAT)).FormatTime().c_str());

		return timelineTimes;
	}
}