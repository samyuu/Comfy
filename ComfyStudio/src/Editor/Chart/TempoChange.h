#pragma once
#include "Timeline/TimelineTick.h"

namespace Comfy::Studio::Editor
{
	struct Tempo
	{
		static constexpr f32 MinBPM = 30.0f;
		static constexpr f32 MaxBPM = 960.0f;

		constexpr Tempo() = default;
		constexpr Tempo(f32 bpm) : BeatsPerMinute(bpm) {}

		f32 BeatsPerMinute = 0.0f;
	};

	struct TempoChange
	{
		static constexpr Tempo DefaultTempo = Tempo(160.0f);

		TempoChange() = default;
		TempoChange(TimelineTick tick, Tempo tempo) : Tick(tick), Tempo(tempo) {}

		constexpr bool operator==(const TempoChange& other) const { return (Tick == other.Tick); }
		constexpr bool operator!=(const TempoChange& other) const { return (Tick != other.Tick); }
		constexpr bool operator<(const TempoChange& other) const { return (Tick < other.Tick); }
		constexpr bool operator>(const TempoChange& other) const { return (Tick > other.Tick); }

		TimelineTick Tick = {};
		Tempo Tempo = {};
	};
}
