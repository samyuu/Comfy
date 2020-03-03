#pragma once
#include "Types.h"

namespace Comfy::Editor
{
	// NOTE: Bar-Beat based BPM depended timeline unit
	struct TimelineTick
	{
		static constexpr int32_t TicksPerBeat = 192;

		static inline TimelineTick ZeroTick() { return TimelineTick(0); };

		static inline TimelineTick FromBeats(int32_t bars) { return FromTicks(TicksPerBeat * bars); };
		static inline TimelineTick FromTicks(int32_t ticks) { return TimelineTick(ticks); };

	public:
		TimelineTick() : tickCount(0) {};
		TimelineTick(int32_t totalTicks) : tickCount(totalTicks) {};

		inline int32_t TotalTicks() const { return tickCount; };
		inline int32_t TotalBeats() const { return TotalTicks() / TicksPerBeat; };

		inline bool operator==(const TimelineTick other) const { return tickCount == other.tickCount; }
		inline bool operator!=(const TimelineTick other) const { return tickCount != other.tickCount; }
		inline bool operator<=(const TimelineTick other) const { return tickCount <= other.tickCount; }
		inline bool operator>=(const TimelineTick other) const { return tickCount >= other.tickCount; }
		inline bool operator<(const TimelineTick other) const { return tickCount < other.tickCount; }
		inline bool operator>(const TimelineTick other) const { return tickCount > other.tickCount; }

	private:
		int32_t tickCount;
	};
}
