#include "SortedTempoMap.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr f64 GetF64FlyingTimeFactorAdjustedBPM(f64 tempoBPM, FlyingTimeFactor flyingTime, bool applyFlyingTimeFactor)
		{
			assert(flyingTime.Factor > 0.0f && tempoBPM > 0.0f);

			return (applyFlyingTimeFactor && flyingTime.Factor != 0.0f) ?
				tempoBPM * static_cast<f64>(flyingTime.Factor) :
				tempoBPM;
		}
	}

	void TempoMapAccelerationStructure::SetApplyFlyingTimeFactor(bool value)
	{
		applyFlyingTimeFactor = value;
	}

	TimeSpan TempoMapAccelerationStructure::ConvertTickToTimeUsingLookupTableIndexing(BeatTick tick) const
	{
		const i32 tickTimeCount = static_cast<i32>(tickTimes.size());
		const i32 totalTicks = tick.Ticks();

		if (totalTicks < 0) // NOTE: Negative tick
		{
			// NOTE: Calculate the duration of a BeatTick at the first tempo
			const TimeSpan firstTickDuration = TimeSpan::FromSeconds((60.0 / firstTempoBPM) / BeatTick::TicksPerBeat);

			// NOTE: Then scale by the negative tick
			return firstTickDuration * totalTicks;
		}
		else if (totalTicks >= tickTimeCount) // NOTE: Tick is outside the defined tempo map
		{
			// NOTE: Take the last calculated time
			const TimeSpan lastTime = GetLastCalculatedTime();

			// NOTE: Calculate the duration of a BeatTick at the last used tempo
			const TimeSpan lastTickDuration = TimeSpan::FromSeconds((60.0 / lastTempoBPM) / BeatTick::TicksPerBeat);

			// NOTE: Then scale by the remaining ticks
			const i32 remainingTicks = (totalTicks - tickTimeCount) + 1;
			return lastTime + (lastTickDuration * remainingTicks);
		}
		else // NOTE: Use the pre calculated lookup table
		{
			return tickTimes.at(totalTicks);
		}
	}

	BeatTick TempoMapAccelerationStructure::ConvertTimeToTickUsingLookupTableBinarySearch(TimeSpan time) const
	{
		const i32 tickTimeCount = static_cast<i32>(tickTimes.size());
		const TimeSpan lastTime = GetLastCalculatedTime();

		if (time < TimeSpan::FromSeconds(0.0)) // NOTE: Negative time
		{
			// NOTE: Calculate the duration of a BeatTick at the first tempo
			const TimeSpan firstTickDuration = TimeSpan::FromSeconds((60.0 / firstTempoBPM) / BeatTick::TicksPerBeat);

			// NOTE: Then the time by the negative tick, this is assuming all tempo changes happen on positive ticks
			return BeatTick(static_cast<i32>(time / firstTickDuration));
		}
		else if (time >= lastTime) // NOTE: Tick is outside the defined tempo map
		{
			const TimeSpan timePastLast = (time - lastTime);

			// NOTE: Each tick past the end has a duration of this value
			const TimeSpan lastTickDuration = TimeSpan::FromSeconds((60.0 / lastTempoBPM) / BeatTick::TicksPerBeat);

			// NOTE: So we just have to divide the remaining ticks by the duration
			const f64 ticks = timePastLast / lastTickDuration;

			// NOTE: And add it to the last tick
			return BeatTick(static_cast<i32>(tickTimeCount + ticks - 1));
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
					return BeatTick::FromTicks(mid);
			}

			return BeatTick::FromTicks((tickTimes[left] - time) < (time - tickTimes[right]) ? left : right);
		}
	}

	TimeSpan TempoMapAccelerationStructure::GetLastCalculatedTime() const
	{
		const size_t tickTimeCount = tickTimes.size();
		return (tickTimeCount == 0) ? TimeSpan::Zero() : tickTimes[tickTimeCount - 1];
	}

	void TempoMapAccelerationStructure::Rebuild(const SortedTempoMap& tempoMap)
	{
		assert(tempoMap.Count() > 0);
		tickTimes.resize(tempoMap.GetRawView().back().Tick.Ticks() + 1);

		f64 lastEndTime = 0.0;
		tempoMap.ForEachNewOrInherited([&](const NewOrInheritedTempoChange& tempoChange)
		{
			const f64 bpm = GetF64FlyingTimeFactorAdjustedBPM(tempoChange.Tempo.BeatsPerMinute, tempoChange.FlyingTime, applyFlyingTimeFactor);
			const f64 beatDuration = (60.0 / bpm);
			const f64 tickDuration = (beatDuration / BeatTick::TicksPerBeat);

			const bool isSingleOrLastTempo = (tempoMap.Count() == 1) || (tempoChange.IndexWithinTempoMap == (tempoMap.Count() - 1));
			const size_t timesCount = (isSingleOrLastTempo) ? (tickTimes.size()) : (tempoMap.GetRawViewAt(tempoChange.IndexWithinTempoMap + 1).Tick.Ticks());

			for (size_t i = 0, t = tempoChange.Tick.Ticks(); t < timesCount; t++)
				tickTimes[t] = TimeSpan::FromSeconds((tickDuration * i++) + lastEndTime);

			if (tempoMap.Count() > 1)
				lastEndTime = tickTimes[timesCount - 1].TotalSeconds() + tickDuration;

			firstTempoBPM = (tempoChange.IndexWithinTempoMap == 0) ? bpm : firstTempoBPM;
			lastTempoBPM = bpm;
			return false;
		});
	}

	SortedTempoMap::SortedTempoMap()
	{
		tempoChanges.push_back(TempoChange(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature));

		accelerationStructure.SetApplyFlyingTimeFactor(false);
		accelerationStructureFlyingTimeFactor.SetApplyFlyingTimeFactor(true);
	}

	void SortedTempoMap::SetTempoChange(TempoChange tempoChangeToInsertOrUpdate)
	{
		assert(tempoChangeToInsertOrUpdate.Tick.Ticks() >= 0);
		if (tempoChangeToInsertOrUpdate.Signature.has_value()) assert(tempoChangeToInsertOrUpdate.Signature->Numerator > 0 && tempoChangeToInsertOrUpdate.Signature->Denominator > 0);

		const auto insertionIndex = InternalFindSortedInsertionIndex(tempoChangeToInsertOrUpdate.Tick);
		if (InBounds(insertionIndex, tempoChanges))
		{
			if (auto& existing = tempoChanges[insertionIndex]; existing.Tick == tempoChangeToInsertOrUpdate.Tick)
				existing = tempoChangeToInsertOrUpdate;
			else
				tempoChanges.insert(tempoChanges.begin() + insertionIndex, tempoChangeToInsertOrUpdate);
		}
		else
		{
			tempoChanges.push_back(tempoChangeToInsertOrUpdate);
		}

		assert(std::is_sorted(tempoChanges.begin(), tempoChanges.end(), [](const auto& a, const auto& b) { return a.Tick < b.Tick; }));
	}

	void SortedTempoMap::RemoveTempoChange(BeatTick tick)
	{
		const auto foundChange = std::find_if(tempoChanges.begin(), tempoChanges.end(), [&](auto& tempoChange) { return tempoChange.Tick == tick; });
		if (foundChange != tempoChanges.end())
			tempoChanges.erase(foundChange);

		// NOTE: Always keep at least one TempoChange because the TimelineMap relies on it
		if (tempoChanges.empty())
			tempoChanges.push_back(TempoChange(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature));
	}

	void SortedTempoMap::ChangeExistingTempoChangeTick(size_t index, BeatTick newTick)
	{
		if (!InBounds(index, tempoChanges))
		{
			assert(false);
			return;
		}

		tempoChanges[index].Tick = newTick;
		assert(std::is_sorted(tempoChanges.begin(), tempoChanges.end(), [](const auto& a, const auto& b) { return a.Tick < b.Tick; }));
	}

	const TempoChange& SortedTempoMap::GetRawViewAt(size_t index) const
	{
		return tempoChanges.at(index);
	}

	const TempoChange& SortedTempoMap::FindRawViewAtTick(BeatTick tick) const
	{
		assert(!tempoChanges.empty());
		if (tempoChanges.size() == 1)
			return tempoChanges.front();

		for (size_t i = 0; i < tempoChanges.size() - 1; i++)
		{
			auto& change = tempoChanges[i];
			auto& nextChange = tempoChanges[i + 1];

			if (change.Tick <= tick && nextChange.Tick > tick)
				return change;
		}

		return tempoChanges.back();
	}

	i32 SortedTempoMap::RawViewToIndex(const TempoChange* tempoChange) const
	{
		if (tempoChange == nullptr)
			return -1;

		if (tempoChange >= &tempoChanges[0] && tempoChange <= &tempoChanges[Count() - 1])
			return static_cast<i32>(std::distance(&tempoChanges[0], tempoChange));

		// NOTE: Accidentally passed a pointer to a stack variable..?
		assert(false);
		return -1;
	}

	NewOrInheritedTempoChange SortedTempoMap::FindNewOrInheritedAt(size_t index) const
	{
		NewOrInheritedTempoChange result = {};
		if (index >= tempoChanges.size())
			return result;

		ForEachNewOrInherited([&](const NewOrInheritedTempoChange& newOrInherited)
		{
			if (newOrInherited.IndexWithinTempoMap == index)
			{
				result = newOrInherited;
				return true;
			}

			return false;
		});

		assert(result.IndexWithinTempoMap == index);
		return result;
	}

	NewOrInheritedTempoChange SortedTempoMap::FindNewOrInheritedAtTick(BeatTick tick) const
	{
		return FindNewOrInheritedAt(RawViewToIndex(&FindRawViewAtTick(tick)));
	}

	const TempoChange* SortedTempoMap::FindNextTempoChangeWithValidSignatureAt(size_t startIndex) const
	{
		if (startIndex == 0)
			return &tempoChanges[startIndex];

		for (size_t i = startIndex; i < tempoChanges.size(); i++)
		{
			if (tempoChanges[i].Signature.has_value())
				return &tempoChanges[i];
		}

		return nullptr;
	}

	size_t SortedTempoMap::Count() const
	{
		return tempoChanges.size();
	}

	void SortedTempoMap::Reset()
	{
		tempoChanges.clear();
		tempoChanges.push_back(TempoChange(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature));
	}

	void SortedTempoMap::RebuildAccelerationStructure()
	{
		accelerationStructure.Rebuild(*this);
		accelerationStructureFlyingTimeFactor.Rebuild(*this);
	}

	TimeSpan SortedTempoMap::TickToTime(BeatTick tick) const
	{
		return accelerationStructure.ConvertTickToTimeUsingLookupTableIndexing(tick);
	}

	BeatTick SortedTempoMap::TimeToTick(TimeSpan tick) const
	{
		return accelerationStructure.ConvertTimeToTickUsingLookupTableBinarySearch(tick);
	}

	TimelineTargetSpawnTimes SortedTempoMap::GetTargetSpawnTimes(const TimelineTarget& target) const
	{
		const BeatTick buttonTick = target.Tick;
		const TimeSpan buttonTime = accelerationStructure.ConvertTickToTimeUsingLookupTableIndexing(buttonTick);

		const TimeSpan buttonTimeAdjusted = accelerationStructureFlyingTimeFactor.ConvertTickToTimeUsingLookupTableIndexing(buttonTick);
		const TimeSpan targetTimeAdjusted = accelerationStructureFlyingTimeFactor.ConvertTickToTimeUsingLookupTableIndexing(buttonTick - BeatTick::FromBars(1));
		const TimeSpan flyingTimeAdjusted = (buttonTimeAdjusted - targetTimeAdjusted);

		const TimeSpan targetTime = (buttonTime - flyingTimeAdjusted);
		const BeatTick targetTick = accelerationStructure.ConvertTimeToTickUsingLookupTableBinarySearch(targetTime);

		return TimelineTargetSpawnTimes { targetTime, buttonTime, targetTick, buttonTick, flyingTimeAdjusted };
	}

	void SortedTempoMap::operator=(std::vector<TempoChange>&& newTempoChanges)
	{
		tempoChanges = std::move(newTempoChanges);
		if (tempoChanges.empty())
			tempoChanges.push_back(TempoChange(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature));
		else
			tempoChanges.front().Tick = BeatTick(0);

		for (auto& tempoChange : tempoChanges)
		{
			if (tempoChange.Tempo.has_value()) assert(tempoChange.Tempo->BeatsPerMinute > 0.0f);
			if (tempoChange.Signature.has_value()) assert(tempoChange.Signature->Numerator > 0 && tempoChange.Signature->Denominator > 0);
		}

		std::sort(tempoChanges.begin(), tempoChanges.end(), [](const auto& a, const auto& b) { return a.Tick < b.Tick; });
	}

	size_t SortedTempoMap::InternalFindSortedInsertionIndex(BeatTick tick) const
	{
		for (size_t i = 0; i < tempoChanges.size(); i++)
		{
			if (tick <= tempoChanges[i].Tick)
				return i;
		}

		return tempoChanges.size();
	}
}
