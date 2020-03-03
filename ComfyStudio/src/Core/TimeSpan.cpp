#include "Types.h"
#include "TimeSpan.h"
#include "DebugStopwatch.h"
#include "Core/Win32/ComfyWindows.h"
#include "Core/Logger.h"

namespace Comfy
{
	namespace
	{
		struct TimingData
		{
			int64_t StartTime;
			int64_t Frequency;

			static int64_t QueryTime()
			{
				LARGE_INTEGER time;
				{
					const BOOL result = ::QueryPerformanceCounter(&time);
					assert(result != 0);
				}
				return time.QuadPart;
			}

			static int64_t QueryFrequency()
			{
				LARGE_INTEGER frequency;
				{
					const BOOL result = ::QueryPerformanceFrequency(&frequency);
					assert(result != 0);
				}
				return frequency.QuadPart;
			}

		} GlobalTimingData;
	}

	void TimeSpan::FormatTime(char* buffer, size_t bufferSize) const
	{
		assert(bufferSize >= RequiredFormatBufferSize);
		const double absoluteTime = glm::abs(timeInSeconds);

		const double minutes = glm::floor(glm::mod(absoluteTime, 3600.0) / 60.0);
		const double seconds = glm::mod(absoluteTime, 60.0);
		const double milliseconds = (seconds - glm::floor(seconds)) * 1000.0;

		const char* signCharacter = (timeInSeconds < 0.0) ? "-" : "";
		sprintf_s(buffer, bufferSize, "%s%02d:%02d.%03d", signCharacter, static_cast<int>(minutes), static_cast<int>(seconds), static_cast<int>(milliseconds));
	}

	std::string TimeSpan::FormatTime() const
	{
		std::string stringBuffer(RequiredFormatBufferSize, '\0');
		{
			FormatTime(stringBuffer.data(), RequiredFormatBufferSize);
		}
		return stringBuffer;
	}

	void TimeSpan::Initialize()
	{
		GlobalTimingData.StartTime = TimingData::QueryTime();
		GlobalTimingData.Frequency = TimingData::QueryFrequency();
	}

	TimeSpan TimeSpan::GetTimeNow()
	{
		const int64_t relativeTime = TimingData::QueryTime() - GlobalTimingData.StartTime;
		return static_cast<double>(relativeTime) / static_cast<double>(GlobalTimingData.Frequency);
	}

	TimeSpan TimeSpan::GetTimeNowAbsolute()
	{
		const int64_t absoluteTime = TimingData::QueryTime();
		return static_cast<double>(absoluteTime) / static_cast<double>(GlobalTimingData.Frequency);
	}

	DebugStopwatch::DebugStopwatch(const char* description)
		: Description(description), TimeOnStart(TimeSpan::GetTimeNow())
	{
	}

	DebugStopwatch::~DebugStopwatch()
	{
		const TimeSpan endTime = TimeSpan::GetTimeNow();
		const TimeSpan elapsed = endTime - TimeOnStart;

		Logger::Log("[DEBUG_STOPWATCH] %s : %.2f MS\n", Description, elapsed.TotalMilliseconds());
	}
}
