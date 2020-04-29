#pragma once
#include "TimeSpan.h"
#include "CoreMacros.h"

#define COMFY_DEBUG_STOPWATCH(description) DebugStopwatch COMFY_UNIQUENAME(__DEBUG_STOPWATCH)(description)

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
