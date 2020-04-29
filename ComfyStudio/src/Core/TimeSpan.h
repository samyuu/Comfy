#pragma once
#include "CoreTypes.h"

namespace Comfy
{
	// NOTE: Time struct storing the underlying time as a double in seconds
	struct TimeSpan
	{
		// NOTE: Constructors
		constexpr TimeSpan() : timeInSeconds(0.0) {}
		explicit constexpr TimeSpan(double seconds) : timeInSeconds(seconds) {}

		// NOTE: Accessers
		inline constexpr double TotalMinutes() const { return TotalSeconds() / 60.0; }
		inline constexpr double TotalSeconds() const { return timeInSeconds; }
		inline constexpr double TotalMilliseconds() const { return TotalSeconds() * 1000.0; }

		// NOTE: Formatting
		void FormatTime(char* buffer, size_t bufferSize) const;
		std::string FormatTime() const;

		// NOTE: (-)mm:ss:fff
		static constexpr size_t RequiredFormatBufferSize = 12;

		// NOTE: Operators
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

		// NOTE: Factory helpers
		static inline constexpr TimeSpan FromMinutes(double value) { return TimeSpan(value * 60.0); }
		static inline constexpr TimeSpan FromSeconds(double value) { return TimeSpan(value); }
		static inline constexpr TimeSpan FromMilliseconds(double value) { return TimeSpan(value / 1000.0); }

		// NOTE: Utilities
		static void Initialize();
		static TimeSpan GetTimeNow();
		static TimeSpan GetTimeNowAbsolute();

	private:
		double timeInSeconds;
	};
}
