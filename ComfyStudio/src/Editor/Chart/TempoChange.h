#pragma once
#include "Types.h"
#include "BeatTick.h"

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

	constexpr bool IsValidTimeSignature(TimeSignature signature)
	{
		return (BeatTick::FromBars(1).TicksPerBeat % signature.Denominator) == 0;
	}

	struct DecomposedTimeSignature { BeatTick TicksPerBeat; i32 BeatsPerBar; };

	constexpr DecomposedTimeSignature DecomposeTimeSignature(TimeSignature signature)
	{
		DecomposedTimeSignature result;
		result.TicksPerBeat = BeatTick::FromBars(1) / signature.Denominator;
		result.BeatsPerBar = signature.Numerator;
		return result;
	}

	// NOTE: The factor that gets applied to a Tempo while calculating the final flying time.
	//		 A factor of 0.5f means "half BPM" and therefore a longer flying time
	//		 and a factor of 2.0f means "double BPM" and therefore a shorter flying time
	struct FlyingTimeFactor
	{
		static constexpr f32 Min = 0.5f;
		static constexpr f32 Max = 4.0f;

		constexpr FlyingTimeFactor() = default;
		constexpr FlyingTimeFactor(f32 factor) : Factor(factor) {}

		constexpr bool operator==(const FlyingTimeFactor& other) const { return (Factor == other.Factor); }
		constexpr bool operator!=(const FlyingTimeFactor& other) const { return (Factor != other.Factor); }

		f32 Factor = 1.0f;
	};

	struct TempoChange
	{
		static constexpr auto DefaultTempo = Tempo(160.0f);
		static constexpr auto DefaultSignature = TimeSignature(4, 4);
		static constexpr auto DefaultFlyingTimeFactor = FlyingTimeFactor(1.0f);

		constexpr TempoChange() = default;
		constexpr TempoChange(BeatTick tick, Tempo tempo, TimeSignature signature) : Tick(tick), Tempo(tempo), Signature(signature) {}
		constexpr TempoChange(BeatTick tick, Tempo tempo, FlyingTimeFactor flyingTimeFactor, TimeSignature signature) : Tick(tick), Tempo(tempo), FlyingTime(flyingTimeFactor), Signature(signature) {}

		constexpr bool operator==(const TempoChange& other) const { return (Tick == other.Tick); }
		constexpr bool operator!=(const TempoChange& other) const { return (Tick != other.Tick); }
		constexpr bool operator<(const TempoChange& other) const { return (Tick < other.Tick); }
		constexpr bool operator>(const TempoChange& other) const { return (Tick > other.Tick); }

		BeatTick Tick = {};
		Tempo Tempo = DefaultTempo;
		FlyingTimeFactor FlyingTime = DefaultFlyingTimeFactor;
		TimeSignature Signature = DefaultSignature;
	};
}
