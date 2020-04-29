#pragma once
#include "Types.h"

namespace Comfy::Editor
{
	// NOTE: Bar-Beat based BPM depended timeline unit
	struct TimelineTick
	{
		static constexpr i32 TicksPerBeat = 192;

		static inline TimelineTick ZeroTick() { return TimelineTick(0); };

		static inline TimelineTick FromBeats(i32 bars) { return FromTicks(TicksPerBeat * bars); };
		static inline TimelineTick FromTicks(i32 ticks) { return TimelineTick(ticks); };

	public:
		TimelineTick() : tickCount(0) {};
		TimelineTick(i32 totalTicks) : tickCount(totalTicks) {};

		inline i32 TotalTicks() const { return tickCount; };
		inline i32 TotalBeats() const { return TotalTicks() / TicksPerBeat; };

		inline bool operator==(const TimelineTick other) const { return tickCount == other.tickCount; }
		inline bool operator!=(const TimelineTick other) const { return tickCount != other.tickCount; }
		inline bool operator<=(const TimelineTick other) const { return tickCount <= other.tickCount; }
		inline bool operator>=(const TimelineTick other) const { return tickCount >= other.tickCount; }
		inline bool operator<(const TimelineTick other) const { return tickCount < other.tickCount; }
		inline bool operator>(const TimelineTick other) const { return tickCount > other.tickCount; }

	private:
		i32 tickCount;
	};
}
