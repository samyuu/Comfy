#include "Types.h"
#include "TimeSpan.h"
#include "Core/Win32/ComfyWindows.h"
#include "Core/Logger.h"

namespace Comfy
{
	namespace
	{
		struct TimingData
		{
			i64 StartTime;
			i64 Frequency;

			static i64 QueryTime()
			{
				LARGE_INTEGER time;
				{
					const BOOL result = ::QueryPerformanceCounter(&time);
					assert(result != 0);
				}
				return time.QuadPart;
			}

			static i64 QueryFrequency()
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

	std::string TimeSpan::ToString() const
	{
		auto stringBuffer = std::string(RequiredFormatBufferSize, '\0');
		FormatTime(stringBuffer.data(), RequiredFormatBufferSize);
		return stringBuffer;
	}

	void TimeSpan::InitializeClock()
	{
		GlobalTimingData.StartTime = TimingData::QueryTime();
		GlobalTimingData.Frequency = TimingData::QueryFrequency();
	}

	TimeSpan TimeSpan::GetTimeNow()
	{
		const i64 relativeTime = TimingData::QueryTime() - GlobalTimingData.StartTime;
		return TimeSpan::FromSeconds(static_cast<double>(relativeTime) / static_cast<double>(GlobalTimingData.Frequency));
	}

	TimeSpan TimeSpan::GetTimeNowAbsolute()
	{
		const i64 absoluteTime = TimingData::QueryTime();
		return TimeSpan::FromSeconds(static_cast<double>(absoluteTime) / static_cast<double>(GlobalTimingData.Frequency));
	}
}
