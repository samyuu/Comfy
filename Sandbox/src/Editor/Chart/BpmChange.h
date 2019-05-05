#pragma once
#include "TimelineTick.h"

// TimelineTick + BPM value struct
// -------------------------------
struct BpmChange
{
	// Timeline Tick
	// -------------
	TimelineTick Tick;

	// Beats Per Minute
	// ----------------
	float Bpm;

	// Constructors / Constructors:
	// ----------------------------
	BpmChange();
	BpmChange(TimelineTick tick, float bpm);
	~BpmChange();
	// ----------------------------
};