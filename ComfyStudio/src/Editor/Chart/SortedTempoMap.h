#pragma once
#include "CoreTypes.h"
#include "TempoChange.h"

namespace Comfy::Studio::Editor
{
	class SortedTempoMap : NonCopyable
	{
	public:
		SortedTempoMap();
		~SortedTempoMap() = default;

	public:
		void SetTempoChange(TimelineTick tick, Tempo tempo, TimeSignature signature);
		void RemoveTempoChange(TimelineTick tick);

		const TempoChange& GetTempoChangeAt(size_t index) const;

		TempoChange& FindTempoChangeAtTick(TimelineTick tick);

		template <typename Func>
		void ForEachBar(Func perBarFunc) const;

		bool FindIsTickOnBar(TimelineTick tick) const;

		size_t TempoChangeCount() const;
		void Clear();

	public:
		auto begin() const { return tempoChanges.cbegin(); }
		auto end() const { return tempoChanges.cend(); }

		void operator=(std::vector<TempoChange>&& newTempoChanges);

	public:
		const std::vector<TempoChange>& GetRawView() const { return tempoChanges; }

	private:
		size_t FindSortedInsertionIndex(TimelineTick tick) const;

	private:
		std::vector<TempoChange> tempoChanges;
	};

	template<typename Func>
	void SortedTempoMap::ForEachBar(Func perBarFunc) const
	{
		for (size_t i = 0, barIndex = 0; i < tempoChanges.size(); i++)
		{
			auto& tempoChange = tempoChanges[i];
			auto* nextTempoChange = IndexOrNull(i + 1, tempoChanges);

			const auto startTick = tempoChange.Tick;
			const auto endTick = (nextTempoChange != nullptr) ? nextTempoChange->Tick : TimelineTick::FromTicks(std::numeric_limits<i32>::max());

			for (auto barTick = startTick; barTick < endTick; barTick += (TimelineTick::FromBeats(1) * tempoChange.Signature.Numerator))
			{
				if (perBarFunc(barTick, barIndex++))
					return;
			}
		}
	}
}
