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

	struct TimeSignature
	{
		static constexpr i16 MinValue = 1;
		static constexpr i16 MaxValue = 64;

		constexpr TimeSignature() = default;
		constexpr TimeSignature(i16 numerator, i16 denominator) : Numerator(numerator), Denominator(denominator) {}

		constexpr bool operator==(const TimeSignature& other) const { return (Numerator == other.Numerator) && (Denominator == other.Denominator); }
		constexpr bool operator!=(const TimeSignature& other) const { return (Numerator != other.Numerator) || (Denominator != other.Denominator); }

		i16 Numerator = MinValue;
		i16 Denominator = MinValue;
	};

	struct DecomposedTimeSignature { TimelineTick TicksPerBeat; i32 BeatsPerBar; };

	constexpr DecomposedTimeSignature DecomposeTimeSignature(TimeSignature signature)
	{
		DecomposedTimeSignature result;
		result.TicksPerBeat = TimelineTick::FromBars(1) / signature.Denominator;
		result.BeatsPerBar = signature.Numerator;
		return result;
	}

	struct TempoChange
	{
		static constexpr auto DefaultTempo = Tempo(160.0f);
		static constexpr auto DefaultSignature = TimeSignature(4, 4);

		TempoChange() = default;
		TempoChange(TimelineTick tick, Tempo tempo, TimeSignature signature) : Tick(tick), Tempo(tempo), Signature(signature) {}

		constexpr bool operator==(const TempoChange& other) const { return (Tick == other.Tick); }
		constexpr bool operator!=(const TempoChange& other) const { return (Tick != other.Tick); }
		constexpr bool operator<(const TempoChange& other) const { return (Tick < other.Tick); }
		constexpr bool operator>(const TempoChange& other) const { return (Tick > other.Tick); }

		TimelineTick Tick = {};
		Tempo Tempo = DefaultTempo;
		TimeSignature Signature = DefaultSignature;
	};
}
