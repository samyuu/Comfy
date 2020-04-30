#pragma once
#include "TimeSpan.h"
#include "CoreMacros.h"

#define COMFY_DEBUG_STOPWATCH(description) auto COMFY_UNIQUENAME(__DEBUG_STOPWATCH) = ::Comfy::DebugStopwatch(description)

namespace Comfy
{
	struct DebugStopwatch
	{
		explicit DebugStopwatch(const char* description) 
			: description(description), timeOnStart(TimeSpan::GetTimeNow())
		{
		}

		~DebugStopwatch()
		{
			const auto endTime = TimeSpan::GetTimeNow();
			const auto elapsed = endTime - timeOnStart;
			DebugLogOnDestruction(elapsed);
		}

	private:
		const char* description;
		TimeSpan timeOnStart;

		void DebugLogOnDestruction(TimeSpan elapsed);
	};
}
