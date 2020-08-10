#include "SortedTempoMap.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	SortedTempoMap::SortedTempoMap()
	{
		SetTempoChange(TimelineTick(0), TempoChange::DefaultTempo);
	}

	void SortedTempoMap::SetTempoChange(TimelineTick tick, Tempo tempo)
	{
		assert(tick.TotalTicks() >= 0);

		if (const auto existing = FindIfOrNull(tempoChanges, [&](const auto& t) { return (t.Tick == tick); }); existing != nullptr)
		{
			existing->Tempo = tempo;
		}
		else
		{
			tempoChanges.emplace_back(tick, tempo);
			std::sort(tempoChanges.begin(), tempoChanges.end());
		}
	}

	void SortedTempoMap::RemoveTempoChange(TimelineTick tick)
	{
		const auto foundChange = std::find_if(tempoChanges.begin(), tempoChanges.end(), [&](auto& tempoChange) { return tempoChange.Tick == tick; });
		if (foundChange != tempoChanges.end())
			tempoChanges.erase(foundChange);

		// NOTE: Always keep at least one TempoChange because the TimelineMap relies on it
		if (tempoChanges.empty())
			SetTempoChange(TimelineTick(0), TempoChange::DefaultTempo);
	}

	TempoChange& SortedTempoMap::GetTempoChangeAt(size_t index)
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
}
