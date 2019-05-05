#pragma once
#include <stdint.h>

// Bar-Beat based BPM depended timeline unit
// -----------------------------------------
struct TimelineTick
{
	// Static Constants:
	// -----------------
	static const int32_t TICKS_PER_BAR = 192 * 4;
	static const int32_t TICKS_PER_BEAT = TICKS_PER_BAR / 4;

	static inline TimelineTick ZeroTick() { return TimelineTick(0); };
	static inline TimelineTick OneBar() { return FromBars(1); };
	static inline TimelineTick FromBars(int32_t bars) { return FromTicks(TICKS_PER_BAR * bars); };
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
	inline int32_t TotalTicks()
	{
		return tickCount;
	}

	// Total bar count
	inline int32_t TotalBars()
	{
		return TotalTicks() / TICKS_PER_BAR;
	}

	// Fraction tick count
	inline int32_t Ticks()
	{
		return TotalTicks() % TICKS_PER_BAR;
	}

	// -------------------

private:
	// Fields:
	// -------
	int32_t tickCount;
};