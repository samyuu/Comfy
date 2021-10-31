#include "Types.h"
#include "TimeSpan.h"
#include "Core/Win32LeanWindowsHeader.h"
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
		const f64 msRoundSeconds = RoundToMilliseconds(Absolute()).timeInSeconds;

		assert(bufferSize >= RequiredFormatBufferSize);
		if (glm::isnan(msRoundSeconds) || glm::isinf(msRoundSeconds))
		{
			strcpy_s(buffer, bufferSize, "--:--.---");
			return;
		}

		constexpr auto maxDisplaybaleTime = TimeSpan::FromSeconds(3599.999999);
		const f64 msRoundSecondsAbs = glm::min(msRoundSeconds, maxDisplaybaleTime.TotalSeconds());
		const f64 min = glm::floor(glm::mod(msRoundSecondsAbs, 3600.0) / 60.0);
		const f64 sec = glm::mod(msRoundSecondsAbs, 60.0);
		const f64 ms = (sec - glm::floor(sec)) * 1000.0;

		const auto signCharacter = (timeInSeconds < 0.0) ? std::array { '-', '\0' } : std::array { '\0', '\0' };
		sprintf_s(buffer, bufferSize, "%s%02d:%02d.%03d",
			signCharacter.data(),
			static_cast<i32>(min),
			static_cast<i32>(sec),
			static_cast<i32>(ms));
	}

	std::array<char, TimeSpan::RequiredFormatBufferSize> TimeSpan::FormatTime() const
	{
		std::array<char, RequiredFormatBufferSize> formatted;
		FormatTimeBuffer(formatted.data(), formatted.size());

		return formatted;
	}

	TimeSpan TimeSpan::ParseFormattedTime(const char* buffer)
	{
		if (buffer == nullptr || buffer[0] == '\0')
			return TimeSpan::Zero();

		bool isNegative = false;
		if (buffer[0] == '+' || buffer[0] == '-')
		{
			isNegative = (buffer[0] == '-');
			buffer++;
		}

		i32 min = 0, sec = 0, ms = 0;
		sscanf_s(buffer, "%02d:%02d.%03d", &min, &sec, &ms);

		min = std::clamp(min, 0, 59);
		sec = std::clamp(sec, 0, 59);
		ms = std::clamp(ms, 0, 999);

		const f64 resultSeconds = (static_cast<f64>(min) * 60.0) + static_cast<f64>(sec) + (static_cast<f64>(ms) * 0.001);
		return TimeSpan(isNegative ? -resultSeconds : resultSeconds);
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
