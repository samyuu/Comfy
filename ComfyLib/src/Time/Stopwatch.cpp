#include "Stopwatch.h"
#include "Core/Logger.h"

namespace Comfy
{
	void DebugStopwatch::DebugLogOnDestruction(TimeSpan elapsed)
	{
		Logger::Log("[DEBUG_STOPWATCH] %s : %.2f MS\n", description, elapsed.TotalMilliseconds());
	}
}
