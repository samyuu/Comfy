#pragma once
#include "Timeline/TimelineTick.h"

namespace Comfy::Studio::Editor
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
		constexpr Tempo() : BeatsPerMinute(0.0f) {}
		constexpr Tempo(float bpm) : BeatsPerMinute(bpm) {}
	};

	// TimelineTick + Tempo value struct
	// ---------------------------------
	struct TempoChange
	{
		static constexpr Tempo DefaultTempo = Tempo(160.0f);

		TempoChange();
		TempoChange(TimelineTick tick, Editor::Tempo tempo);

		TimelineTick Tick;
		Tempo Tempo;

		inline bool operator==(const TempoChange &other) const { return (Tick == other.Tick); }
		inline bool operator<(const TempoChange &other) const { return Tick < other.Tick; }
		inline bool operator>(const TempoChange &other) const { return Tick > other.Tick; }
	};
}
