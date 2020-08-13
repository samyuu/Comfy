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
		
		size_t TempoChangeCount() const;

	private:
		std::vector<TempoChange> tempoChanges;
	};
}
