#include "TempoMap.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	TempoMap::TempoMap()
	{
		SetTempoChange(TimelineTick(0), TempoChange::DefaultTempo);
	}

	TempoMap::~TempoMap()
	{
	}

	void TempoMap::SetTempoChange(TimelineTick tick, Tempo tempo)
	{
		assert(tick.TotalTicks() >= 0);

		// Look for an existing TempoChange at the input tick
		TempoChange* existingTempoChange = nullptr;
		for (size_t i = 0; i < TempoChangeCount(); i++)
		{
			TempoChange &tempoChange = GetTempoChangeAt(i);
			if (tempoChange.Tick == tick)
				existingTempoChange = &tempoChange;
		}
		
		// to then either update it
		if (existingTempoChange != nullptr)
		{
			existingTempoChange->Tempo = tempo;
		}
		else // or add a new TempoChange
		{
			tempoChanges.emplace_back(tick, tempo);
			std::sort(tempoChanges.begin(), tempoChanges.end());
		}
	}

	void TempoMap::RemoveTempoChange(TimelineTick tick)
	{
		for (auto iterator = tempoChanges.begin(); iterator != tempoChanges.end(); ++iterator)
		{
			if (iterator->Tick == tick)
			{
				tempoChanges.erase(iterator);
				break;
			}
		}

		// Always keep at least one TempoChange because the TimelineMap relies on it
		if (TempoChangeCount() < 1)
			SetTempoChange(TimelineTick(0), TempoChange::DefaultTempo);
	}

	TempoChange& TempoMap::GetTempoChangeAt(size_t index)
	{
		return tempoChanges.at(index);
	}

	size_t TempoMap::TempoChangeCount() const
	{
		return tempoChanges.size();
	}
}
