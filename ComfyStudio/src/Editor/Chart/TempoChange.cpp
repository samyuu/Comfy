#include "TempoChange.h"

namespace Editor
{
	TempoChange::TempoChange() : Tick(0), Tempo(0.0f)
	{

	}

	TempoChange::TempoChange(TimelineTick tick, Editor::Tempo tempo) : Tick(tick), Tempo(tempo)
	{

	}
}