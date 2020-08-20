#include "SortedTempoMap.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	SortedTempoMap::SortedTempoMap()
	{
		tempoChanges.emplace_back(TimelineTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
	}

	void SortedTempoMap::SetTempoChange(TimelineTick tick, Tempo tempo, TimeSignature signature)
	{
		assert(tick.TotalTicks() >= 0);

		const auto insertionIndex = FindSortedInsertionIndex(tick);
		if (InBounds(insertionIndex, tempoChanges))
		{
			if (auto& existing = tempoChanges[insertionIndex]; existing.Tick == tick)
			{
				existing.Tempo = tempo;
				existing.Signature = signature;
			}
			else
			{
				tempoChanges.emplace(tempoChanges.begin() + insertionIndex, tick, tempo, signature);
			}
		}
		else
		{
			tempoChanges.emplace_back(tick, tempo, signature);
		}

		assert(std::is_sorted(tempoChanges.begin(), tempoChanges.end(), [](const auto& a, const auto& b) { return a.Tick < b.Tick; }));
	}

	void SortedTempoMap::RemoveTempoChange(TimelineTick tick)
	{
		const auto foundChange = std::find_if(tempoChanges.begin(), tempoChanges.end(), [&](auto& tempoChange) { return tempoChange.Tick == tick; });
		if (foundChange != tempoChanges.end())
			tempoChanges.erase(foundChange);

		// NOTE: Always keep at least one TempoChange because the TimelineMap relies on it
		if (tempoChanges.empty())
			tempoChanges.emplace_back(TimelineTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
	}

	const TempoChange& SortedTempoMap::GetTempoChangeAt(size_t index) const
	{
		return tempoChanges.at(index);
	}

	TempoChange& SortedTempoMap::FindTempoChangeAtTick(TimelineTick tick)
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

	size_t SortedTempoMap::TempoChangeCount() const
	{
		return tempoChanges.size();
	}

	void SortedTempoMap::Clear()
	{
		tempoChanges.clear();
		tempoChanges.emplace_back(TimelineTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
	}

	size_t SortedTempoMap::FindSortedInsertionIndex(TimelineTick tick) const
	{
		for (size_t i = 0; i < tempoChanges.size(); i++)
		{
			if (tick <= tempoChanges[i].Tick)
				return i;
		}

		return tempoChanges.size();
	}
}
