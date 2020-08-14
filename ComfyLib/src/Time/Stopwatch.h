#pragma once
#include "TimeSpan.h"
#include "CoreMacros.h"

#define COMFY_DEBUG_STOPWATCH(description) auto COMFY_UNIQUENAME(__DEBUG_STOPWATCH) = ::Comfy::DebugStopwatch(description)

namespace Comfy
{
	struct Stopwatch
	{
	public:
		Stopwatch() = default;
		~Stopwatch() = default;

	public:
		static Stopwatch StartNew();

	public:
		void Start();
		TimeSpan Restart();
		TimeSpan Stop();

		bool IsRunning() const;
		TimeSpan GetElapsed() const;

	private:
		bool isRunning = false;
		TimeSpan timeOnStart;
	};

	struct DebugStopwatch
	{
		explicit DebugStopwatch(const char* description)
			: description(description), stopwatch(Stopwatch::StartNew())
		{
		}

		~DebugStopwatch()
		{
			const auto elapsed = stopwatch.GetElapsed();
			DebugLogOnDestruction(elapsed);
		}

	private:
		const char* description;
		Stopwatch stopwatch;

		void DebugLogOnDestruction(TimeSpan elapsed);
	};
}
