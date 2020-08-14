#include "Stopwatch.h"
#include "Core/Logger.h"

namespace Comfy
{
	Stopwatch Stopwatch::StartNew()
	{
		auto stopwatch = Stopwatch();
		stopwatch.Start();
		return stopwatch;
	}

	void Stopwatch::Start()
	{
		if (isRunning)
			return;

		timeOnStart = TimeSpan::GetTimeNow();
		isRunning = true;
	}

	TimeSpan Stopwatch::Restart()
	{
		const auto elapsed = Stop();
		Start();
		return elapsed;
	}

	TimeSpan Stopwatch::Stop()
	{
		const auto elapsed = GetElapsed();
		isRunning = false;
		timeOnStart = TimeSpan::Zero();
		return elapsed;
	}

	bool Stopwatch::IsRunning() const
	{
		return isRunning;
	}

	TimeSpan Stopwatch::GetElapsed() const
	{
		if (!isRunning)
			return TimeSpan::Zero();

		const auto endTime = TimeSpan::GetTimeNow();
		const auto elapsedSinceStart = endTime - timeOnStart;

		return elapsedSinceStart;
	}

	void DebugStopwatch::DebugLogOnDestruction(TimeSpan elapsed)
	{
		Logger::Log("[DEBUG_STOPWATCH] %s : %.2f MS\n", description, elapsed.TotalMilliseconds());
	}
}
