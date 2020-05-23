#pragma once
#include "CoreTypes.h"
#include "TempoChange.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Collection of tempo changes always kept in TimelineTick order
	class TempoMap
	{
	public:
		TempoMap();
		~TempoMap() = default;

	public:
		void SetTempoChange(TimelineTick tick, Tempo tempo);
		void RemoveTempoChange(TimelineTick tick);
		
		TempoChange& GetTempoChangeAt(size_t index);
		size_t TempoChangeCount() const;

	private:
		std::vector<TempoChange> tempoChanges;
	};
}
