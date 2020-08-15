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
		if (glm::isnan(timeInSeconds) || glm::isinf(timeInSeconds))
		{
			strcpy_s(buffer, bufferSize, "--:--.---");
			return;
		}

		const f64 absoluteTime = glm::abs(timeInSeconds);

		const f64 minutes = glm::floor(glm::mod(absoluteTime, 3600.0) / 60.0);
		const f64 seconds = glm::mod(absoluteTime, 60.0);
		const f64 milliseconds = (seconds - glm::floor(seconds)) * 1000.0;

		const char* signCharacter = (timeInSeconds < 0.0) ? "-" : "";
		sprintf_s(buffer, bufferSize, "%s%02d:%02d.%03d", signCharacter, static_cast<i32>(minutes), static_cast<i32>(seconds), static_cast<i32>(milliseconds));
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
		return TimeSpan::FromSeconds(static_cast<f64>(relativeTime) / static_cast<f64>(GlobalTimingData.Frequency));
	}

	TimeSpan TimeSpan::GetTimeNowAbsolute()
	{
		assert(GlobalTimingData.Initialized);
		const i64 absoluteTime = TimingData::QueryTime();
		return TimeSpan::FromSeconds(static_cast<f64>(absoluteTime) / static_cast<f64>(GlobalTimingData.Frequency));
	}
}
