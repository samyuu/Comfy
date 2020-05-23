#pragma once
#include "Types.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Bar-Beat based BPM depended timeline unit
	struct TimelineTick
	{
	public:
		static constexpr i32 TicksPerBeat = 192;
		static constexpr TimelineTick ZeroTick() { return TimelineTick(0); }
		static constexpr TimelineTick FromBeats(i32 bars) { return FromTicks(TicksPerBeat * bars); }
		static constexpr TimelineTick FromTicks(i32 ticks) { return TimelineTick(ticks); }

	public:
		constexpr TimelineTick() : tickCount(0) {}
		constexpr TimelineTick(i32 totalTicks) : tickCount(totalTicks) {}

		constexpr i32 TotalTicks() const { return tickCount; }
		constexpr i32 TotalBeats() const { return TotalTicks() / TicksPerBeat; }

		constexpr bool operator==(const TimelineTick other) const { return tickCount == other.tickCount; }
		constexpr bool operator!=(const TimelineTick other) const { return tickCount != other.tickCount; }
		constexpr bool operator<=(const TimelineTick other) const { return tickCount <= other.tickCount; }
		constexpr bool operator>=(const TimelineTick other) const { return tickCount >= other.tickCount; }
		constexpr bool operator<(const TimelineTick other) const { return tickCount < other.tickCount; }
		constexpr bool operator>(const TimelineTick other) const { return tickCount > other.tickCount; }

	private:
		i32 tickCount;
	};
}
