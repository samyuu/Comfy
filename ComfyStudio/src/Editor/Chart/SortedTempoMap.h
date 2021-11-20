#pragma once
#include "Types.h"
#include "BeatTick.h"
#include "SortedTargetList.h"
#include "Time/TimeSpan.h"
#include <optional>

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

	struct Tempo
	{
		static constexpr f32 MinBPM = 30.0f;
		static constexpr f32 MaxBPM = 960.0f;

		constexpr Tempo() = default;
		constexpr Tempo(f32 bpm) : BeatsPerMinute(bpm) {}

		f32 BeatsPerMinute = 0.0f;
	};

	struct TimeSignature
	{
		static constexpr i16 MinValue = 1;
		static constexpr i16 MaxValue = 64;

		constexpr TimeSignature() = default;
		constexpr TimeSignature(i16 numerator, i16 denominator) : Numerator(numerator), Denominator(denominator) {}

		constexpr bool operator==(const TimeSignature& other) const { return (Numerator == other.Numerator) && (Denominator == other.Denominator); }
		constexpr bool operator!=(const TimeSignature& other) const { return (Numerator != other.Numerator) || (Denominator != other.Denominator); }

		i16 Numerator = MinValue;
		i16 Denominator = MinValue;
	};

	constexpr bool IsValidTimeSignature(TimeSignature signature)
	{
		return (BeatTick::FromBars(1).TicksPerBeat % signature.Denominator) == 0;
	}

	struct DecomposedTimeSignature { BeatTick TicksPerBeat; i32 BeatsPerBar; };

	constexpr DecomposedTimeSignature DecomposeTimeSignature(TimeSignature signature)
	{
		DecomposedTimeSignature result;
		result.TicksPerBeat = BeatTick::FromBars(1) / signature.Denominator;
		result.BeatsPerBar = signature.Numerator;
		return result;
	}

	// NOTE: The factor that gets applied to a Tempo while calculating the final flying time.
	//		 A factor of 0.5f means "half BPM" and therefore a longer flying time
	//		 and a factor of 2.0f means "double BPM" and therefore a shorter flying time
	struct FlyingTimeFactor
	{
		static constexpr f32 Min = 0.5f;
		static constexpr f32 Max = 4.0f;

		constexpr FlyingTimeFactor() = default;
		constexpr FlyingTimeFactor(f32 factor) : Factor(factor) {}

		constexpr bool operator==(const FlyingTimeFactor& other) const { return (Factor == other.Factor); }
		constexpr bool operator!=(const FlyingTimeFactor& other) const { return (Factor != other.Factor); }

		f32 Factor = 1.0f;
	};

	struct TempoChange
	{
		static constexpr auto DefaultTempo = Tempo(160.0f);
		static constexpr auto DefaultSignature = TimeSignature(4, 4);
		static constexpr auto DefaultFlyingTimeFactor = FlyingTimeFactor(1.0f);

		constexpr TempoChange() = default;
		constexpr TempoChange(BeatTick tick, std::optional<Tempo> tempo, std::optional<TimeSignature> signature) : Tick(tick), Tempo(tempo), Signature(signature) {}
		constexpr TempoChange(BeatTick tick, std::optional<Tempo> tempo, std::optional<FlyingTimeFactor> flyingTime, std::optional<TimeSignature> signature) : Tick(tick), Tempo(tempo), FlyingTime(flyingTime), Signature(signature) {}

		// TODO: The equallity operators especially are questionable
		constexpr bool operator==(const TempoChange& other) const { return (Tick == other.Tick); }
		constexpr bool operator!=(const TempoChange& other) const { return (Tick != other.Tick); }
		constexpr bool operator<(const TempoChange& other) const { return (Tick < other.Tick); }
		constexpr bool operator>(const TempoChange& other) const { return (Tick > other.Tick); }

		BeatTick Tick = {};
		std::optional<Tempo> Tempo = {};
		std::optional<FlyingTimeFactor> FlyingTime = {};
		std::optional<TimeSignature> Signature = {};
	};

	// NOTE: Specifically to pass back to the user code inside template ForEach functions
	struct NewOrInheritedTempoChange
	{
		BeatTick Tick;
		Tempo Tempo;
		FlyingTimeFactor FlyingTime;
		TimeSignature Signature;
		size_t IndexWithinTempoMap;
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
		void ChangeExistingTempoChangeTick(size_t index, BeatTick newTick);

		// NOTE: Return const references instead of copies to allow taking the address of
		const TempoChange& GetRawViewAt(size_t index) const;
		const TempoChange& FindRawViewAtTick(BeatTick tick) const;
		i32 RawViewToIndex(const TempoChange* tempoChange) const;

		// NOTE: Named "Find" to make clear that these are a O(N) operations and only provided for convenience.
		//		 Call ForEachNewOrInheritedInRange() instead of this if it is intended to be used inside a for loop
		NewOrInheritedTempoChange FindNewOrInheritedAt(size_t index) const;
		NewOrInheritedTempoChange FindNewOrInheritedAtTick(BeatTick tick) const;

		// NOTE: Mainly for skipping past TempoChanges without tempo signatures while iterating over bars/beats
		const TempoChange* FindNextTempoChangeWithValidSignatureAt(size_t startIndex) const;

		size_t Count() const;
		void Reset();

		// NOTE: Needs to be called every time a TempoChange has been edited
		void RebuildAccelerationStructure();

		TimeSpan TickToTime(BeatTick tick) const;
		BeatTick TimeToTick(TimeSpan tick) const;

		// NOTE: Adjusted for FlyingTimeFactor. Should be used instead of "buttonTick - BeatTick::FromBars(1)" calculations whenever appropriate
		TimelineTargetSpawnTimes GetTargetSpawnTimes(const TimelineTarget& target) const;

	public:
		template <typename Func>
		void ForEachNewOrInheritedInRange(size_t start, size_t end, Func perTempoChangeFunc) const;

		template <typename Func>
		void ForEachNewOrInherited(Func perTempoChangeFunc) const;

		template <typename Func>
		void ForEachBar(Func perBarFunc) const;

		template <typename Func>
		void ForEachBeatBar(Func perBeatBarFunc) const;

	public:
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
	void SortedTempoMap::ForEachNewOrInheritedInRange(size_t start, size_t end, Func perTempoChangeFunc) const
	{
		assert(start < tempoChanges.size() && end <= tempoChanges.size() && !tempoChanges.empty());

		NewOrInheritedTempoChange newOrInherited = {};
		newOrInherited.Tempo = tempoChanges[0].Tempo.value_or(TempoChange::DefaultTempo);
		newOrInherited.FlyingTime = tempoChanges[0].FlyingTime.value_or(TempoChange::DefaultFlyingTimeFactor);
		newOrInherited.Signature = tempoChanges[0].Signature.value_or(TempoChange::DefaultSignature);

		for (size_t i = 0; i < end; i++)
		{
			const auto& tempoChange = tempoChanges[i];
			newOrInherited.Tick = tempoChange.Tick;
			newOrInherited.Tempo = tempoChange.Tempo.value_or(newOrInherited.Tempo);
			newOrInherited.FlyingTime = tempoChange.FlyingTime.value_or(newOrInherited.FlyingTime);
			newOrInherited.Signature = tempoChange.Signature.value_or(newOrInherited.Signature);
			newOrInherited.IndexWithinTempoMap = i;

			if (i >= start)
			{
				if (perTempoChangeFunc(newOrInherited))
					return;
			}
		}
	}

	template<typename Func>
	void SortedTempoMap::ForEachNewOrInherited(Func perTempoChangeFunc) const
	{
		return ForEachNewOrInheritedInRange(static_cast<size_t>(0), tempoChanges.size(), perTempoChangeFunc);
	}

	template<typename Func>
	void SortedTempoMap::ForEachBar(Func perBarFunc) const
	{
		assert(!tempoChanges.empty());
		TimeSignature newOrInheritedSignature = tempoChanges[0].Signature.value_or(TempoChange::DefaultSignature);

		for (size_t tempoIndex = 0, barIndex = 0; tempoIndex < tempoChanges.size(); /*tempoIndex++*/)
		{
			const TempoChange* tempoChange = FindNextTempoChangeWithValidSignatureAt(tempoIndex);
			const TempoChange* nextTempoChange = FindNextTempoChangeWithValidSignatureAt(tempoIndex + 1);
			assert(tempoChange != nullptr);

			const BeatTick startTick = tempoChange->Tick;
			const BeatTick endTick = (nextTempoChange != nullptr) ? nextTempoChange->Tick : BeatTick::FromTicks(std::numeric_limits<i32>::max());

			newOrInheritedSignature = tempoChange->Signature.value_or(newOrInheritedSignature);

			const auto[ticksPerBeat, beatsPerBar] = DecomposeTimeSignature(newOrInheritedSignature);
			for (auto barTick = startTick; barTick < endTick; barTick += (ticksPerBeat * beatsPerBar))
			{
				if (perBarFunc(barTick, barIndex++))
					return;
			}

			if (nextTempoChange == nullptr)
				return;

			const i32 nextIndexAfterSkippingNonSignatureChanges = RawViewToIndex(nextTempoChange);
			assert(nextIndexAfterSkippingNonSignatureChanges > tempoIndex);
			tempoIndex = nextIndexAfterSkippingNonSignatureChanges;
		}
	}

	template <typename Func>
	void SortedTempoMap::ForEachBeatBar(Func perBeatBarFunc) const
	{
		assert(!tempoChanges.empty());
		TimeSignature newOrInheritedSignature = tempoChanges[0].Signature.value_or(TempoChange::DefaultSignature);

		for (size_t tempoIndex = 0, barIndex = 0; tempoIndex < tempoChanges.size(); /*tempoIndex++*/)
		{
			const TempoChange* tempoChange = FindNextTempoChangeWithValidSignatureAt(tempoIndex);
			const TempoChange* nextTempoChange = FindNextTempoChangeWithValidSignatureAt(tempoIndex + 1);
			assert(tempoChange != nullptr);

			const BeatTick startTick = tempoChange->Tick;
			const BeatTick endTick = (nextTempoChange != nullptr) ? nextTempoChange->Tick : BeatTick::FromTicks(std::numeric_limits<i32>::max());

			newOrInheritedSignature = tempoChange->Signature.value_or(newOrInheritedSignature);

			const auto[ticksPerBeat, beatsPerBar] = DecomposeTimeSignature(newOrInheritedSignature);
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

			if (nextTempoChange == nullptr)
				return;

			const i32 nextIndexAfterSkippingNonSignatureChanges = RawViewToIndex(nextTempoChange);
			assert(nextIndexAfterSkippingNonSignatureChanges > tempoIndex);
			tempoIndex = nextIndexAfterSkippingNonSignatureChanges;
		}
	}
}
