#pragma once
#include "Types.h"
#include "TempoChange.h"

namespace Comfy::Studio::Editor
{
	class SortedTempoMap : NonCopyable
	{
	public:
		SortedTempoMap();
		~SortedTempoMap() = default;

	public:
		void SetTempoChange(BeatTick tick, Tempo tempo, TimeSignature signature);
		void RemoveTempoChange(BeatTick tick);

		const TempoChange& GetTempoChangeAt(size_t index) const;

		TempoChange& FindTempoChangeAtTick(BeatTick tick);

		template <typename Func>
		void ForEachBar(Func perBarFunc) const;

		template <typename Func>
		void ForEachBeatBar(Func perBeatBarFunc) const;

		size_t TempoChangeCount() const;
		void Clear();

	public:
		auto begin() const { return tempoChanges.cbegin(); }
		auto end() const { return tempoChanges.cend(); }

		void operator=(std::vector<TempoChange>&& newTempoChanges);

	public:
		const std::vector<TempoChange>& GetRawView() const { return tempoChanges; }

	private:
		size_t FindSortedInsertionIndex(BeatTick tick) const;

	private:
		std::vector<TempoChange> tempoChanges;
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
