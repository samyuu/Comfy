#pragma once
#include <stdint.h>

namespace Editor
{
	// Bar-Beat based BPM depended timeline unit
	// -----------------------------------------
	struct TimelineTick
	{
		// Static Constants:
		// -----------------
		static const int32_t TICKS_PER_BEAT = 192;

		static inline TimelineTick ZeroTick() { return TimelineTick(0); };

		static inline TimelineTick FromBeats(int32_t bars) { return FromTicks(TICKS_PER_BEAT * bars); };
		static inline TimelineTick FromTicks(int32_t ticks) { return TimelineTick(ticks); };

	public:
		// Constructors / Destructors:
		// ---------------------------
		TimelineTick() : tickCount(0) {};
		TimelineTick(int32_t totalTicks) : tickCount(totalTicks) {}
		// ---------------------------

		// Conversion Methods:
		// -------------------

		// Total tick count
		inline int32_t TotalTicks() const
		{
			return tickCount;
		}

		// Total beat count
		inline int32_t TotalBeats() const
		{
			return TotalTicks() / TICKS_PER_BEAT;
		}

		// -------------------

		// Operators:
		// ----------
		inline bool operator== (const TimelineTick other) const { return tickCount == other.tickCount; }
		inline bool operator!= (const TimelineTick other) const { return tickCount != other.tickCount; }
		inline bool operator<= (const TimelineTick other) const { return tickCount <= other.tickCount; }
		inline bool operator>= (const TimelineTick other) const { return tickCount >= other.tickCount; }
		inline bool operator< (const TimelineTick other) const { return tickCount < other.tickCount; }
		inline bool operator> (const TimelineTick other) const { return tickCount > other.tickCount; }
		// ----------

	private:
		// Fields:
		// -------
		int32_t tickCount;
	};
}