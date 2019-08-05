#include "TimelineMap.h"
#include "Input/Keyboard.h"
#include <assert.h>

namespace Editor
{
	TimelineMap::TimelineMap()
	{
	}

	TimelineMap::TimelineMap(std::vector<TimeSpan>& times, Tempo firstTempo, Tempo lastTempo) : tickTimes(times), firstTempo(firstTempo), lastTempo(lastTempo)
	{
	}

	TimeSpan TimelineMap::GetTimeAt(TimelineTick tick) const
	{
		int32_t tickTimeCount = static_cast<int32_t>(tickTimes.size());

		if (tick.TotalTicks() < 0) // negative tick
		{
			// calculate the duration of a TimelineTick at the first tempo
			TimeSpan firstTickDuration = ((60.0 / firstTempo.BeatsPerMinute) / TimelineTick::TICKS_PER_BEAT);

			// then scale by the negative tick
			return firstTickDuration * tick.TotalTicks();
		}
		else if (tick.TotalTicks() >= tickTimeCount) // tick is outside the defined tempo map
		{
			// take the last calculated time
			TimeSpan lastTime = GetLastCalculatedTime();;

			// calculate the duration of a TimelineTick at the last used tempo
			TimeSpan lastTickDuration = ((60.0 / lastTempo.BeatsPerMinute) / TimelineTick::TICKS_PER_BEAT);

			// then scale by the remaining ticks
			int remainingTicks = tick.TotalTicks() - tickTimeCount;
			return lastTime + (lastTickDuration * remainingTicks);
		}
		else // use the pre calculated lookup table
		{
			return tickTimes.at(tick.TotalTicks());
		}
	}

	TimeSpan TimelineMap::GetLastCalculatedTime() const
	{
		size_t tickTimeCount = tickTimes.size();
		return tickTimeCount == 0 ? TimeSpan(0.0) : tickTimes[tickTimeCount - 1];
	}

	TimelineTick TimelineMap::GetTickAt(TimeSpan time) const
	{
		int32_t tickTimeCount = static_cast<int32_t>(tickTimes.size());
		TimeSpan lastTime = GetLastCalculatedTime();

		if (time < 0.0) // negative time
		{
			// calculate the duration of a TimelineTick at the first tempo
			TimeSpan firstTickDuration = ((60.0 / firstTempo.BeatsPerMinute) / TimelineTick::TICKS_PER_BEAT);

			// then the time by the negative tick, this is assuming all tempo changes happen on positive ticks
			return TimelineTick(static_cast<int32_t>(time / firstTickDuration));
		}
		else if (time >= lastTime) // tick is outside the defined tempo map
		{
			TimeSpan timePastLast = time - lastTime;

			// each tick past the end has a duration of this value
			TimeSpan lastTickDuration = ((60.0 / lastTempo.BeatsPerMinute) / TimelineTick::TICKS_PER_BEAT);

			// so we just have to divide the remaining ticks by the duration
			double ticks = timePastLast / lastTickDuration;
			// and add it to the last tick
			return TimelineTick(static_cast<int32_t>(tickTimeCount + ticks));
		}
		else // perform a binary search
		{
			int left = 0, right = tickTimeCount - 1;

			while (left <= right)
			{
				int mid = (left + right) / 2;

				if (time < tickTimes[mid])
					right = mid - 1;
				else if (time > tickTimes[mid])
					left = mid + 1;
				else
					return mid;
			}

			return (tickTimes[left] - time) < (time - tickTimes[right]) ? left : right;
		}
	}

	void TimelineMap::CalculateMapTimes(TempoMap& tempoMap)
	{
		assert(tempoMap.TempoChangeCount() > 0);

		TempoChange& lastTempoChange = tempoMap.GetTempoChangeAt(tempoMap.TempoChangeCount() - 1);
		const size_t timeCount = lastTempoChange.Tick.TotalTicks();

		tickTimes.resize(timeCount);
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

				if (tempoChanges > 1)
					tempoChangeEndTime = tickTimes[timesCount - 1].TotalSeconds() + tickDuration;
			}
		}

		firstTempo = tempoMap.GetTempoChangeAt(0).Tempo;
		lastTempo = lastTempoChange.Tempo;
	}
}