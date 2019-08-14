#include "TimeSpan.h"
#include "glfw/glfw3.h"
#include "Logger.h"

TimeSpan TimeSpan::GetTimeNow()
{
	return glfwGetTime();
}

DebugStopwatch::DebugStopwatch(const char* description) 
	: Description(description), TimeOnStart(TimeSpan::GetTimeNow())
{
}

DebugStopwatch::~DebugStopwatch()
{
	TimeSpan endTime = TimeSpan::GetTimeNow();
	TimeSpan elapsed = endTime - TimeOnStart;

	Logger::Log("[DEBUG_STOPWATCH] %s : %.2f MS\n", Description, elapsed.TotalMilliseconds());
}
