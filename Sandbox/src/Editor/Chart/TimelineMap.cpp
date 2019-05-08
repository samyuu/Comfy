#include "TimelineMap.h"

namespace Editor
{
	TimelineMap::TimelineMap()
	{
	}

	TimelineMap::TimelineMap(std::vector<TimeSpan>& times) : tickTimes(times)
	{
	}

	TimeSpan TimelineMap::GetTimeAt(TimelineTick tick)
	{
		int tickTimeCount = tickTimes.size();

		if (tick.TotalTicks() < 0) // negative tick
		{
			// take the first calculated time
			TimeSpan firstTime = tickTimes.at(0);
			TimeSpan secondTime = tickTimes.at(1);

			// calculate the duration of a TimelineTick at the first tempo
			TimeSpan tickDuration = firstTime - secondTime;

			// then scale by the negative tick
			return tickDuration * tick.TotalTicks();
		}
		else if (tick.TotalTicks() >= tickTimeCount) // tick is outside the tempo map
		{
			// take the last calculated time
			TimeSpan lastTime = tickTimes.at(tickTimeCount - 1);
			TimeSpan secondToLast = tickTimes.at(tickTimeCount - 2);

			// calculate the duration of a TimelineTick at the last used tempo
			TimeSpan tickDuration = secondToLast - lastTime;

			// then scale by the remaining ticks
			int remainingTicks = tick.TotalTicks() - tickTimeCount + 1;
			return lastTime + (tickDuration * remainingTicks);
		}
		else // use the pre calculated lookup table
		{
			return tickTimes.at(tick.TotalTicks());
		}
	}

	TimelineTick TimelineMap::GetTickAt(TimeSpan time)
	{
		TimeSpan lastTime = tickTimes[tickTimes.size() - 1];

		if (time >= lastTime)
		{

		}
		else
		{
			// perform a binary search

		}

		assert(false);
		return 0.0;
	}

	TimelineMap TimelineMap::CalculateMapTimes(TempoMap& tempoMap)
	{
		assert(tempoMap.TempoChangeCount() > 0);

		TempoChange& lastTempoChange = tempoMap.GetTempoChangeAt(tempoMap.TempoChangeCount() - 1);
		const size_t timeCount = lastTempoChange.Tick.TotalTicks();

		std::vector<TimeSpan> tickTimes(timeCount);
		{
			// the time of when the last tempo change ended, so we can use higher precision multiplication
			double tempoChangeEndTime = 0.0;

			const size_t tempoChanges = tempoMap.TempoChangeCount();
			for (size_t b = 0; b < tempoChanges; b++)
			{
				TempoChange& tempoChange = tempoMap.GetTempoChangeAt(b);

				const double beatDuration = (60.0 / tempoChange.Tempo.BeatsPerMinute);
				const double tickDuration = (beatDuration / TimelineTick::TICKS_PER_BEAT);

				bool onlyTempo = (tempoChanges == 1);
				bool lastTempo = (b == (tempoChanges - 1));

				const size_t timesStart = tempoChange.Tick.TotalTicks();
				const size_t timesCount = (onlyTempo || lastTempo) ? (tickTimes.size()) : (tempoMap.GetTempoChangeAt(b + 1).Tick.TotalTicks());

				for (size_t i = 0, t = timesStart; t < timesCount; t++)
					tickTimes[t] = (tickDuration * i++) + tempoChangeEndTime;

				tempoChangeEndTime = tickTimes[timesCount - 1].TotalSeconds() + tickDuration;
			}
		}

		return TimelineMap(tickTimes);
	}
}