#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy
{
	// TODO: Store underlying tick as i64 with a focus on a *very large* fixed point (10^12 (?))
	//		 Make entirely constexpr compliant
	// NOTE: Time struct storing the underlying time as a double in seconds
	struct TimeSpan
	{
	public:
		static constexpr frame_t DefaultFrameRate = 60.0f;

	public:
		constexpr TimeSpan() : timeInSeconds(0.0) {}
		constexpr explicit TimeSpan(f64 seconds) : timeInSeconds(seconds) {}

	public:
		static constexpr TimeSpan Zero() { return TimeSpan(0.0); }
		static constexpr TimeSpan FromMinutes(f64 value) { return TimeSpan(value * 60.0); }
		static constexpr TimeSpan FromSeconds(f64 value) { return TimeSpan(value); }
		static constexpr TimeSpan FromMilliseconds(f64 value) { return TimeSpan(value / 1000.0); }
		static constexpr TimeSpan FromFrames(frame_t frames, frame_t frameRate = DefaultFrameRate) { return FromSeconds(frames / frameRate); }

	public:
		constexpr f64 TotalMinutes() const { return TotalSeconds() / 60.0; }
		constexpr f64 TotalSeconds() const { return timeInSeconds; }
		constexpr f64 TotalMilliseconds() const { return TotalSeconds() * 1000.0; }
		constexpr frame_t ToFrames(frame_t frameRate = DefaultFrameRate) const { return static_cast<frame_t>(TotalSeconds() * frameRate); }

	public:
		// NOTE: Enough to store "(-)mm:ss.fff"
		static constexpr size_t RequiredFormatBufferSize = 12;

		void FormatTimeBuffer(char* buffer, size_t bufferSize) const;
		std::array<char, RequiredFormatBufferSize> FormatTime() const;

	public:
		constexpr bool operator==(const TimeSpan& other) const { return timeInSeconds == other.timeInSeconds; }
		constexpr bool operator!=(const TimeSpan& other) const { return timeInSeconds != other.timeInSeconds; }
		constexpr bool operator<=(const TimeSpan& other) const { return timeInSeconds <= other.timeInSeconds; }
		constexpr bool operator>=(const TimeSpan& other) const { return timeInSeconds >= other.timeInSeconds; }
		constexpr bool operator<(const TimeSpan& other) const { return timeInSeconds < other.timeInSeconds; }
		constexpr bool operator>(const TimeSpan& other) const { return timeInSeconds > other.timeInSeconds; }

		constexpr TimeSpan operator+(const TimeSpan other) const { return FromSeconds(TotalSeconds() + other.TotalSeconds()); }
		constexpr TimeSpan operator-(const TimeSpan other) const { return FromSeconds(TotalSeconds() - other.TotalSeconds()); }

		constexpr TimeSpan& operator+=(const TimeSpan& other) { timeInSeconds += other.timeInSeconds; return *this; }
		constexpr TimeSpan& operator-=(const TimeSpan& other) { timeInSeconds -= other.timeInSeconds; return *this; }

		constexpr TimeSpan operator*(f64 other) const { return FromSeconds(TotalSeconds() * other); }
		constexpr TimeSpan operator*(i32 other) const { return FromSeconds(TotalSeconds() * other); }

		constexpr f64 operator/(TimeSpan other) const { return TotalSeconds() / other.TotalSeconds(); }
		constexpr f64 operator/(f64 other) const { return TotalSeconds() / other; }
		constexpr f64 operator/(i32 other) const { return TotalSeconds() / other; }

		constexpr TimeSpan operator+() const { return TimeSpan(+timeInSeconds); }
		constexpr TimeSpan operator-() const { return TimeSpan(-timeInSeconds); }

	public:
		// NOTE: Must be called once before GetTimeNow() / GetTimeNowAbsolute() can be called
		static void InitializeClock();

		static TimeSpan GetTimeNow();
		static TimeSpan GetTimeNowAbsolute();

	private:
		f64 timeInSeconds;
	};
}
