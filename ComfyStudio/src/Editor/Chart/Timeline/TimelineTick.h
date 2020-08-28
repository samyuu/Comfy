#pragma once
#include "Types.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Beat tick based BPM depended fixed point timeline unit
	struct TimelineTick
	{
	public:
		// NOTE: All code should be built around this value in a way that changing it won't break any behavior other than the maximum timeline precision
		static constexpr i32 TicksPerBeat = (192 / 4);

		static constexpr TimelineTick Zero() { return TimelineTick(0); }
		static constexpr TimelineTick FromTicks(i32 ticks) { return TimelineTick(ticks); }
		static constexpr TimelineTick FromBeats(i32 beats) { return FromTicks(TicksPerBeat * beats); }
		static constexpr TimelineTick FromBars(i32 bars, i32 beatsPerBar = 4) { return FromBeats(bars * beatsPerBar); }

	public:
		constexpr TimelineTick() : tickCount(0) {}
		constexpr explicit TimelineTick(i32 ticks) : tickCount(ticks) {}

		constexpr i32 Ticks() const { return tickCount; }
		constexpr f32 BeatsFraction() const { return static_cast<f32>(tickCount) / static_cast<f32>(TicksPerBeat); }

		constexpr bool operator==(const TimelineTick other) const { return tickCount == other.tickCount; }
		constexpr bool operator!=(const TimelineTick other) const { return tickCount != other.tickCount; }
		constexpr bool operator<=(const TimelineTick other) const { return tickCount <= other.tickCount; }
		constexpr bool operator>=(const TimelineTick other) const { return tickCount >= other.tickCount; }
		constexpr bool operator<(const TimelineTick other) const { return tickCount < other.tickCount; }
		constexpr bool operator>(const TimelineTick other) const { return tickCount > other.tickCount; }

		constexpr TimelineTick operator+(const TimelineTick other) const { return TimelineTick(tickCount + other.tickCount); }
		constexpr TimelineTick operator-(const TimelineTick other) const { return TimelineTick(tickCount - other.tickCount); }
		constexpr TimelineTick operator*(const i32 ticks) const { return TimelineTick(tickCount * ticks); }
		constexpr TimelineTick operator/(const i32 ticks) const { return TimelineTick(tickCount / ticks); }
		constexpr TimelineTick operator%(const TimelineTick other) const { return TimelineTick(tickCount % other.tickCount); }

		constexpr TimelineTick& operator+=(const TimelineTick other) { (tickCount += other.tickCount); return *this; }
		constexpr TimelineTick& operator-=(const TimelineTick other) { (tickCount -= other.tickCount); return *this; }
		constexpr TimelineTick& operator*=(const i32 ticks) { (tickCount *= ticks); return *this; }
		constexpr TimelineTick& operator/=(const i32 ticks) { (tickCount *= ticks); return *this; }
		constexpr TimelineTick& operator%=(const TimelineTick other) { (tickCount %= other.tickCount); return *this; }

		constexpr TimelineTick operator-() const { return TimelineTick(-tickCount); }

	private:
		i32 tickCount;
	};
}
