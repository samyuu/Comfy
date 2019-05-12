#pragma once
#include "TimelineTick.h"

namespace Editor
{
	constexpr float MIN_BPM = 2.0f;
	constexpr float MAX_BPM = 960.0f;

	// Wrapper struct around a BPM value
	// ---------------------------------
	struct Tempo
	{
		float BeatsPerMinute;

		// Constructors / Deconstructors:
		// ------------------------------
		constexpr Tempo() : BeatsPerMinute(0.0f) {};
		constexpr Tempo(float bpm) : BeatsPerMinute(bpm) {};
	};

	constexpr Tempo DEFAULT_TEMPO = Tempo(120.0f);

	// TimelineTick + Tempo value struct
	// ---------------------------------
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

		// Operators:
		// ----------
		inline bool operator== (const TempoChange &other) const { return (Tick == other.Tick) && (Tick == other.Tick); };
		inline bool operator< (const TempoChange &other) const { return Tick < other.Tick; };
		inline bool operator> (const TempoChange &other) const { return Tick > other.Tick; };
		// ----------
	};
}