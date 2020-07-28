#include "TimelineMap.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	TimelineMap::TimelineMap(std::vector<TimeSpan>& times, Tempo firstTempo, Tempo lastTempo)
		: tickTimes(times), firstTempo(firstTempo), lastTempo(lastTempo)
	{
	}

	TimeSpan TimelineMap::GetTimeAt(TimelineTick tick) const
	{
		const i32 tickTimeCount = static_cast<i32>(tickTimes.size());

		if (tick.TotalTicks() < 0) // NOTE: Negative tick
		{
			// NOTE: Calculate the duration of a TimelineTick at the first tempo
			const TimeSpan firstTickDuration = TimeSpan::FromSeconds((60.0 / firstTempo.BeatsPerMinute) / TimelineTick::TicksPerBeat);

			// NOTE: Then scale by the negative tick
			return firstTickDuration * tick.TotalTicks();
		}
		else if (tick.TotalTicks() >= tickTimeCount) // NOTE: Tick is outside the defined tempo map
		{
			// NOTE: Take the last calculated time
			const TimeSpan lastTime = GetLastCalculatedTime();

			// NOTE: Calculate the duration of a TimelineTick at the last used tempo
			const TimeSpan lastTickDuration = TimeSpan::FromSeconds((60.0 / lastTempo.BeatsPerMinute) / TimelineTick::TicksPerBeat);

			// NOTE: Then scale by the remaining ticks
			const i32 remainingTicks = tick.TotalTicks() - tickTimeCount;
			return lastTime + (lastTickDuration * remainingTicks);
		}
		else // NOTE: Use the pre calculated lookup table
		{
			return tickTimes.at(tick.TotalTicks());
		}
	}

	TimeSpan TimelineMap::GetLastCalculatedTime() const
	{
		const size_t tickTimeCount = tickTimes.size();
		return tickTimeCount == 0 ? TimeSpan(0.0) : tickTimes[tickTimeCount - 1];
	}

	TimelineTick TimelineMap::GetTickAt(TimeSpan time) const
	{
		const i32 tickTimeCount = static_cast<i32>(tickTimes.size());
		const TimeSpan lastTime = GetLastCalculatedTime();

		if (time < TimeSpan::FromSeconds(0.0)) // NOTE: Negative time
		{
			// NOTE: Calculate the duration of a TimelineTick at the first tempo
			const TimeSpan firstTickDuration = TimeSpan::FromSeconds((60.0 / firstTempo.BeatsPerMinute) / TimelineTick::TicksPerBeat);

			// NOTE: Then the time by the negative tick, this is assuming all tempo changes happen on positive ticks
			return TimelineTick(static_cast<i32>(time / firstTickDuration));
		}
		else if (time >= lastTime) // NOTE: Tick is outside the defined tempo map
		{
			const TimeSpan timePastLast = (time - lastTime);

			// NOTE: Each tick past the end has a duration of this value
			const TimeSpan lastTickDuration = TimeSpan::FromSeconds((60.0 / lastTempo.BeatsPerMinute) / TimelineTick::TicksPerBeat);

			// NOTE: So we just have to divide the remaining ticks by the duration
			const double ticks = timePastLast / lastTickDuration;
			// NOTE: And add it to the last tick
			return TimelineTick(static_cast<i32>(tickTimeCount + ticks));
		}
		else // NOTE: Perform a binary search
		{
			i32 left = 0, right = tickTimeCount - 1;

			while (left <= right)
			{
				const i32 mid = (left + right) / 2;

				if (time < tickTimes[mid])
					right = mid - 1;
				else if (time > tickTimes[mid])
					left = mid + 1;
				else
					return TimelineTick::FromTicks(mid);
			}

			return TimelineTick::FromTicks((tickTimes[left] - time) < (time - tickTimes[right]) ? left : right);
		}
	}

	TimelineTick TimelineMap::GetTickAtFixedTempo(TimeSpan time, Tempo tempo) const
	{
		const auto firstTickDuration = TimeSpan::FromSeconds((60.0 / firstTempo.BeatsPerMinute) / TimelineTick::TicksPerBeat);
		return TimelineTick(static_cast<i32>(time / firstTickDuration));
	}

	void TimelineMap::CalculateMapTimes(SortedTempoMap& tempoMap)
	{
		assert(tempoMap.TempoChangeCount() > 0);

		TempoChange& lastTempoChange = tempoMap.GetTempoChangeAt(tempoMap.TempoChangeCount() - 1);
		const size_t timeCount = lastTempoChange.Tick.TotalTicks();

		tickTimes.resize(timeCount);
		{
			// NOTE: The time of when the last tempo change ended, so we can use higher precision multiplication
			double tempoChangeEndTime = 0.0;

			const size_t tempoChangeCount = tempoMap.TempoChangeCount();
			for (size_t tempoIndex = 0; tempoIndex < tempoChangeCount; tempoIndex++)
			{
				TempoChange& tempoChange = tempoMap.GetTempoChangeAt(tempoIndex);

				const double beatDuration = (60.0 / tempoChange.Tempo.BeatsPerMinute);
				const double tickDuration = (beatDuration / TimelineTick::TicksPerBeat);

				const bool singleTempo = (tempoChangeCount == 1);
				const bool isLastTempo = (tempoIndex == (tempoChangeCount - 1));

				const size_t timesStart = tempoChange.Tick.TotalTicks();
				const size_t timesCount = (singleTempo || isLastTempo) ? (tickTimes.size()) : (tempoMap.GetTempoChangeAt(tempoIndex + 1).Tick.TotalTicks());

				for (size_t i = 0, t = timesStart; t < timesCount; t++)
					tickTimes[t] = TimeSpan::FromSeconds((tickDuration * i++) + tempoChangeEndTime);

				if (tempoChangeCount > 1)
					tempoChangeEndTime = tickTimes[timesCount - 1].TotalSeconds() + tickDuration;
			}
		}

		firstTempo = tempoMap.GetTempoChangeAt(0).Tempo;
		lastTempo = lastTempoChange.Tempo;
	}
}
