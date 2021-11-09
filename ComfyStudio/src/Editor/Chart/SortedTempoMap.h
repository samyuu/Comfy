#pragma once
#include "Types.h"
#include "TempoChange.h"
#include "SortedTargetList.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Maybe this shouldn't be defined inside *this* header but it's the easiest for now
	struct TimelineTargetSpawnTimes
	{
		TimeSpan TargetTime;
		TimeSpan ButtonTime;
		BeatTick TargetTick;
		BeatTick ButtonTick;
		TimeSpan FlyingTime;
	};

	class SortedTempoMap;

	// NOTE: Basically an acceleration structure for quickly doing Time <-> Beat conversions.
	//		 Probably shouldn't be accessed by user code directly
	class TempoMapAccelerationStructure : NonCopyable
	{
	public:
		TempoMapAccelerationStructure() = default;
		~TempoMapAccelerationStructure() = default;

	public:
		void SetApplyFlyingTimeFactor(bool value);

		TimeSpan ConvertTickToTimeUsingLookupTableIndexing(BeatTick tick) const;
		BeatTick ConvertTimeToTickUsingLookupTableBinarySearch(TimeSpan time) const;

		TimeSpan GetLastCalculatedTime() const;
		void Rebuild(const SortedTempoMap& tempoMap);

	private:
		// NOTE: Pre calculated tick times up to the last tempo change
		std::vector<TimeSpan> tickTimes;
		f64 firstTempoBPM = 0.0, lastTempoBPM = 0.0;
		bool applyFlyingTimeFactor = false;
	};

	class SortedTempoMap : NonCopyable
	{
	public:
		SortedTempoMap();
		~SortedTempoMap() = default;

	public:
		void SetTempoChange(TempoChange tempoChangeToInsertOrUpdate);
		void RemoveTempoChange(BeatTick tick);

		// NOTE: Assumes the new tick to be within the bounds of the previous and next tempo change
		void UpdateTempoChangeTick(size_t index, BeatTick newTick);

		const TempoChange& GetTempoChangeAt(size_t index) const;
		const TempoChange& FindTempoChangeAtTick(BeatTick tick) const;

		// NOTE: Needs to be called every time a TempoChange has been edited
		void RebuildAccelerationStructure();

		size_t TempoChangeCount() const;
		void Clear();

		TimeSpan TickToTime(BeatTick tick) const;
		BeatTick TimeToTick(TimeSpan tick) const;

		// NOTE: Adjusted for FlyingTimeFactor. Should be used instead of "buttonTick - BeatTick::FromBars(1)" calculations whenever appropriate
		TimelineTargetSpawnTimes GetTargetSpawnTimes(const TimelineTarget& target) const;

		template <typename Func>
		void ForEachBar(Func perBarFunc) const;

		template <typename Func>
		void ForEachBeatBar(Func perBeatBarFunc) const;

	public:
		auto begin() const { return tempoChanges.cbegin(); }
		auto end() const { return tempoChanges.cend(); }

		// TODO: Maybe replace with explicit "MoveAssignRawVectorForDeserialization()" function or something similar..?
		void operator=(std::vector<TempoChange>&& newTempoChanges);

		const std::vector<TempoChange>& GetRawView() const { return tempoChanges; }

	private:
		size_t InternalFindSortedInsertionIndex(BeatTick tick) const;

	private:
		std::vector<TempoChange> tempoChanges;
		TempoMapAccelerationStructure accelerationStructure;
		TempoMapAccelerationStructure accelerationStructureFlyingTimeFactor;
	};

	template<typename Func>
	void SortedTempoMap::ForEachBar(Func perBarFunc) const
	{
		for (size_t i = 0, barIndex = 0; i < tempoChanges.size(); i++)
		{
			const auto& tempoChange = tempoChanges[i];
			const auto* nextTempoChange = IndexOrNull(i + 1, tempoChanges);

			const auto startTick = tempoChange.Tick;
			const auto endTick = (nextTempoChange != nullptr) ? nextTempoChange->Tick : BeatTick::FromTicks(std::numeric_limits<i32>::max());

			const auto[ticksPerBeat, beatsPerBar] = DecomposeTimeSignature(tempoChange.Signature);
			for (auto barTick = startTick; barTick < endTick; barTick += (ticksPerBeat * beatsPerBar))
			{
				if (perBarFunc(barTick, barIndex++))
					return;
			}
		}
	}

	template <typename Func>
	void SortedTempoMap::ForEachBeatBar(Func perBeatBarFunc) const
	{
		for (size_t i = 0, barIndex = 0; i < tempoChanges.size(); i++)
		{
			const auto& tempoChange = tempoChanges[i];
			const auto* nextTempoChange = IndexOrNull(i + 1, tempoChanges);

			const auto startTick = tempoChange.Tick;
			const auto endTick = (nextTempoChange != nullptr) ? nextTempoChange->Tick : BeatTick::FromTicks(std::numeric_limits<i32>::max());

			const auto[ticksPerBeat, beatsPerBar] = DecomposeTimeSignature(tempoChange.Signature);
			for (auto beatTick = startTick; beatTick < endTick; beatTick += ticksPerBeat)
			{
				if (perBeatBarFunc(beatTick, barIndex, true))
					return;

				for (auto beatIndexWithinBar = 1; (beatIndexWithinBar < beatsPerBar) && (beatTick + ticksPerBeat < endTick); beatIndexWithinBar++)
				{
					beatTick += ticksPerBeat;
					if (perBeatBarFunc(beatTick, barIndex, false))
						return;
				}

				barIndex++;
			}
		}
	}
}
