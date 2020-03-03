#pragma once
#include "TimeSpan.h"

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define uniquename(prefix) CONCAT(prefix, __COUNTER__)
#define DEBUG_STOPWATCH(description) DebugStopwatch uniquename(__DEBUG_STOPWATCH)(description)

namespace Comfy
{
	struct DebugStopwatch
	{
		DebugStopwatch(const char* description);
		~DebugStopwatch();

		const char* Description;
		TimeSpan TimeOnStart;
	};
}
