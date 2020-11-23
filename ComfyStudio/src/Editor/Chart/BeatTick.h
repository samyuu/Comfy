#pragma once
#include "Types.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Beat tick based BPM depended fixed point timeline unit
	struct BeatTick
	{
	public:
		// NOTE: All code should optimally be built around this value in a way that changing it won't break any behavior other than the maximum timeline precision
		static constexpr i32 TicksPerBeat = (192 / 4);

		static constexpr BeatTick Zero() { return BeatTick(0); }
		static constexpr BeatTick FromTicks(i32 ticks) { return BeatTick(ticks); }
		static constexpr BeatTick FromBeats(i32 beats) { return FromTicks(TicksPerBeat * beats); }
		static constexpr BeatTick FromBars(i32 bars, i32 beatsPerBar = 4) { return FromBeats(bars * beatsPerBar); }

	public:
		constexpr BeatTick() : tickCount(0) {}
		constexpr explicit BeatTick(i32 ticks) : tickCount(ticks) {}

		constexpr i32 Ticks() const { return tickCount; }
		constexpr f32 BeatsFraction() const { return static_cast<f32>(tickCount) / static_cast<f32>(TicksPerBeat); }

		constexpr bool operator==(const BeatTick other) const { return tickCount == other.tickCount; }
		constexpr bool operator!=(const BeatTick other) const { return tickCount != other.tickCount; }
		constexpr bool operator<=(const BeatTick other) const { return tickCount <= other.tickCount; }
		constexpr bool operator>=(const BeatTick other) const { return tickCount >= other.tickCount; }
		constexpr bool operator<(const BeatTick other) const { return tickCount < other.tickCount; }
		constexpr bool operator>(const BeatTick other) const { return tickCount > other.tickCount; }

		constexpr BeatTick operator+(const BeatTick other) const { return BeatTick(tickCount + other.tickCount); }
		constexpr BeatTick operator-(const BeatTick other) const { return BeatTick(tickCount - other.tickCount); }
		constexpr BeatTick operator*(const i32 ticks) const { return BeatTick(tickCount * ticks); }
		constexpr BeatTick operator/(const i32 ticks) const { return BeatTick(tickCount / ticks); }
		constexpr BeatTick operator%(const BeatTick other) const { return BeatTick(tickCount % other.tickCount); }

		constexpr BeatTick& operator+=(const BeatTick other) { (tickCount += other.tickCount); return *this; }
		constexpr BeatTick& operator-=(const BeatTick other) { (tickCount -= other.tickCount); return *this; }
		constexpr BeatTick& operator*=(const i32 ticks) { (tickCount *= ticks); return *this; }
		constexpr BeatTick& operator/=(const i32 ticks) { (tickCount *= ticks); return *this; }
		constexpr BeatTick& operator%=(const BeatTick other) { (tickCount %= other.tickCount); return *this; }

		constexpr BeatTick operator-() const { return BeatTick(-tickCount); }

	private:
		i32 tickCount;
	};
}
