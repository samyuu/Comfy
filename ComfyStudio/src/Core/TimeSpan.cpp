#include "Types.h"
#include "TimeSpan.h"
#include "DebugStopwatch.h"
#include "glfw/glfw3.h"
#include "Logger.h"

void TimeSpan::FormatTime(char* buffer, size_t bufferSize) const
{
	double absoluteTime = glm::abs(time);
	double minutes = glm::floor(glm::mod(absoluteTime, 3600.0) / 60.0);
	double seconds = glm::mod(absoluteTime, 60.0);
	double milliseconds = (seconds - glm::floor(seconds)) * 1000.0;
	
	const char* sign = (time < 0.0) ? "-" : "";

	sprintf_s(buffer, bufferSize, "%s%02d:%02d.%03d", sign, static_cast<int>(minutes), static_cast<int>(seconds), static_cast<int>(milliseconds));
}

String TimeSpan::FormatTime() const
{
	char buffer[16];
	FormatTime(buffer, sizeof(buffer));

	return String(buffer);
}

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
