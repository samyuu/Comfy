#pragma once
#include "TimelineTick.h"

namespace Editor
{
	constexpr float MAX_BPM = 960.0f;
	constexpr float MIN_BPM = 2.0f;

	struct Tempo
	{
		float BeatsPerMinute;

		// Constructors / Deconstructors:
		// ------------------------------
		Tempo() : BeatsPerMinute(0.0f) {};
		Tempo(float bpm) : BeatsPerMinute(bpm) {};
	};

	// TimelineTick + Tempo value struct
	// -------------------------------
	struct TempoChange
	{
		// Timeline Tick
		// -------------
		TimelineTick Tick;

		// Tempo
		// -----
		Tempo Tempo;

		// Constructors / Deconstructors:
		// ------------------------------
		TempoChange();
		TempoChange(TimelineTick tick, Editor::Tempo tempo);
	};
}