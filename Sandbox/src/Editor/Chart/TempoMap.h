#pragma once
#include "TempoChange.h"
#include <vector>
#include <assert.h>

namespace Editor
{
	// Collection of tempo changes always kept in TimelineTick order
	// -------------------------------------------------------------
	class TempoMap
	{
	public:
		TempoMap();
		~TempoMap();

		inline void Add(TempoChange tempoChange)
		{
			assert(tempoChange.Tick.TotalTicks() >= 0);
			// TODO: insert / sort
			tempoChanges.push_back(tempoChange);
		}

		inline TempoChange& GetTempoChangeAt(size_t index)
		{
			return tempoChanges.at(index);
		}

		inline size_t TempoChangeCount()
		{
			return tempoChanges.size();
		}

	private:
		std::vector<TempoChange> tempoChanges;
	};
}