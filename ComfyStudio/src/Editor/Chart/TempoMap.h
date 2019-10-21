#pragma once
#include "TempoChange.h"
#include "Core/CoreTypes.h"

namespace Editor
{
	// Collection of tempo changes always kept in TimelineTick order
	// -------------------------------------------------------------
	class TempoMap
	{
	public:
		TempoMap();
		~TempoMap();

		void SetTempoChange(TimelineTick tick, Tempo tempo);
		void RemoveTempoChange(TimelineTick tick);
		TempoChange& GetTempoChangeAt(size_t index);
		size_t TempoChangeCount() const;

	private:
		std::vector<TempoChange> tempoChanges;
	};
}