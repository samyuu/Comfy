#include "SortedTempoMap.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	SortedTempoMap::SortedTempoMap()
	{
		tempoChanges.emplace_back(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
	}

	void SortedTempoMap::SetTempoChange(BeatTick tick, Tempo tempo, TimeSignature signature)
	{
		assert(tick.Ticks() >= 0);
		assert(signature.Numerator > 0 && signature.Denominator > 0);

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

	void SortedTempoMap::RemoveTempoChange(BeatTick tick)
	{
		const auto foundChange = std::find_if(tempoChanges.begin(), tempoChanges.end(), [&](auto& tempoChange) { return tempoChange.Tick == tick; });
		if (foundChange != tempoChanges.end())
			tempoChanges.erase(foundChange);

		// NOTE: Always keep at least one TempoChange because the TimelineMap relies on it
		if (tempoChanges.empty())
			tempoChanges.emplace_back(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
	}

	const TempoChange& SortedTempoMap::GetTempoChangeAt(size_t index) const
	{
		return tempoChanges.at(index);
	}

	TempoChange& SortedTempoMap::FindTempoChangeAtTick(BeatTick tick)
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
		tempoChanges.emplace_back(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
	}

	void SortedTempoMap::operator=(std::vector<TempoChange>&& newTempoChanges)
	{
		tempoChanges = std::move(newTempoChanges);
		if (tempoChanges.empty())
			tempoChanges.emplace_back(BeatTick(0), TempoChange::DefaultTempo, TempoChange::DefaultSignature);
		else
			tempoChanges.front().Tick = BeatTick(0);

		for (auto& tempoChange : tempoChanges)
		{
			assert(tempoChange.Tempo.BeatsPerMinute > 0.0f);
			assert(tempoChange.Signature.Numerator > 0 && tempoChange.Signature.Denominator > 0);
		}

		std::sort(tempoChanges.begin(), tempoChanges.end(), [](const auto& a, const auto& b) { return a.Tick < b.Tick; });
	}

	size_t SortedTempoMap::FindSortedInsertionIndex(BeatTick tick) const
	{
		for (size_t i = 0; i < tempoChanges.size(); i++)
		{
			if (tick <= tempoChanges[i].Tick)
				return i;
		}

		return tempoChanges.size();
	}
}
