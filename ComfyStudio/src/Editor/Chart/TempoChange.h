#pragma once
#include "Timeline/TimelineTick.h"

namespace Comfy::Studio::Editor
{
	struct Tempo
	{
	public:
		static constexpr float MinBPM = 2.0f;
		static constexpr float MaxBPM = 960.0f;

	public:
		constexpr Tempo() : BeatsPerMinute(0.0f) {}
		constexpr Tempo(float bpm) : BeatsPerMinute(bpm) {}

	public:
		float BeatsPerMinute;
	};

	struct TempoChange
	{
	public:
		static constexpr Tempo DefaultTempo = Tempo(160.0f);

	public:
		TempoChange() = default;
		TempoChange(TimelineTick tick, Tempo tempo) : Tick(tick), Tempo(tempo) {}

		bool operator==(const TempoChange& other) const { return (Tick == other.Tick); }
		bool operator!=(const TempoChange& other) const { return (Tick != other.Tick); }
		bool operator<(const TempoChange& other) const { return (Tick < other.Tick); }
		bool operator>(const TempoChange& other) const { return (Tick > other.Tick); }

	public:
		TimelineTick Tick = {};
		Tempo Tempo = {};
	};
}
