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
		constexpr TimeSpan() : timeInSeconds(0.0) {}
		explicit constexpr TimeSpan(double seconds) : timeInSeconds(seconds) {}

	public:
		inline constexpr double TotalMinutes() const { return TotalSeconds() / 60.0; }
		inline constexpr double TotalSeconds() const { return timeInSeconds; }
		inline constexpr double TotalMilliseconds() const { return TotalSeconds() * 1000.0; }

	public:
		// NOTE: Enough to store "(-)mm:ss:fff"
		static constexpr size_t RequiredFormatBufferSize = 12;

		void FormatTime(char* buffer, size_t bufferSize) const;		

		// NOTE: Should be small enough for SSO and not allocate additional memory
		std::string ToString() const;

	public:
		inline bool operator==(const TimeSpan& other) const { return timeInSeconds == other.timeInSeconds; }
		inline bool operator!=(const TimeSpan& other) const { return timeInSeconds != other.timeInSeconds; }
		inline bool operator<=(const TimeSpan& other) const { return timeInSeconds <= other.timeInSeconds; }
		inline bool operator>=(const TimeSpan& other) const { return timeInSeconds >= other.timeInSeconds; }
		inline bool operator<(const TimeSpan& other) const { return timeInSeconds < other.timeInSeconds; }
		inline bool operator>(const TimeSpan& other) const { return timeInSeconds > other.timeInSeconds; }

		inline TimeSpan operator+(const TimeSpan other) const { return FromSeconds(TotalSeconds() + other.TotalSeconds()); }
		inline TimeSpan operator-(const TimeSpan other) const { return FromSeconds(TotalSeconds() - other.TotalSeconds()); }

		inline TimeSpan& operator+=(const TimeSpan& other) { this->timeInSeconds += other.timeInSeconds; return *this; }
		inline TimeSpan& operator-=(const TimeSpan& other) { this->timeInSeconds -= other.timeInSeconds; return *this; }

		inline TimeSpan operator*(double other) const { return FromSeconds(TotalSeconds() * other); }
		inline TimeSpan operator*(int other) const { return FromSeconds(TotalSeconds() * other); }

		inline double operator/(TimeSpan other) const { return TotalSeconds() / other.TotalSeconds(); }
		inline double operator/(double other) const { return TotalSeconds() / other; }
		inline double operator/(int other) const { return TotalSeconds() / other; }

		inline TimeSpan operator-() const { return TimeSpan(-timeInSeconds); }

	public:
		static inline constexpr TimeSpan FromMinutes(double value) { return TimeSpan(value * 60.0); }
		static inline constexpr TimeSpan FromSeconds(double value) { return TimeSpan(value); }
		static inline constexpr TimeSpan FromMilliseconds(double value) { return TimeSpan(value / 1000.0); }

	public:
		// NOTE: Must be called once before GetTimeNow() / GetTimeNowAbsolute() can be called
		static void InitializeClock();

		static TimeSpan GetTimeNow();
		static TimeSpan GetTimeNowAbsolute();

	private:
		double timeInSeconds;
	};
}
