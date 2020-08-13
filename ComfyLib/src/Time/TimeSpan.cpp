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
			bool Initialized = false;
			i64 StartTime = 0;
			i64 Frequency = 0;

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

	void TimeSpan::FormatTimeBuffer(char* buffer, size_t bufferSize) const
	{
		assert(bufferSize >= RequiredFormatBufferSize);
		const double absoluteTime = glm::abs(timeInSeconds);

		const double minutes = glm::floor(glm::mod(absoluteTime, 3600.0) / 60.0);
		const double seconds = glm::mod(absoluteTime, 60.0);
		const double milliseconds = (seconds - glm::floor(seconds)) * 1000.0;

		const char* signCharacter = (timeInSeconds < 0.0) ? "-" : "";
		sprintf_s(buffer, bufferSize, "%s%02d:%02d.%03d", signCharacter, static_cast<int>(minutes), static_cast<int>(seconds), static_cast<int>(milliseconds));
	}

	std::array<char, TimeSpan::RequiredFormatBufferSize> TimeSpan::FormatTime() const
	{
		std::array<char, RequiredFormatBufferSize> formatted;
		FormatTimeBuffer(formatted.data(), formatted.size());

		return formatted;
	}

	void TimeSpan::InitializeClock()
	{
		assert(!GlobalTimingData.Initialized);
		GlobalTimingData.StartTime = TimingData::QueryTime();
		GlobalTimingData.Frequency = TimingData::QueryFrequency();
		GlobalTimingData.Initialized = true;
	}

	TimeSpan TimeSpan::GetTimeNow()
	{
		assert(GlobalTimingData.Initialized);
		const i64 relativeTime = TimingData::QueryTime() - GlobalTimingData.StartTime;
		return TimeSpan::FromSeconds(static_cast<double>(relativeTime) / static_cast<double>(GlobalTimingData.Frequency));
	}

	TimeSpan TimeSpan::GetTimeNowAbsolute()
	{
		assert(GlobalTimingData.Initialized);
		const i64 absoluteTime = TimingData::QueryTime();
		return TimeSpan::FromSeconds(static_cast<double>(absoluteTime) / static_cast<double>(GlobalTimingData.Frequency));
	}
}
