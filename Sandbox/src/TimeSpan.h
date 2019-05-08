#pragma once
#include <string>

// Time struct storing the underlying Time as a double in Seconds
// --------------------------------------------------------------
struct TimeSpan
{
	// Constructos:
	// ------------
	inline TimeSpan() : time(0) {}
	inline TimeSpan(double seconds) : time(seconds) {}
	// ------------

	inline double TotalMinutes()
	{
		return TotalSeconds() / 60.0;
	}

	inline double TotalSeconds()
	{
		return time;
	}

	inline double TotalMilliseconds()
	{
		return TotalSeconds() * 1000.0;
	}

	static inline TimeSpan FromMinutes(double value)
	{
		return TimeSpan(value * 60.0);
	}

	static inline TimeSpan FromSeconds(double value) 
	{
		return TimeSpan(value);
	};

	static inline TimeSpan FromMilliseconds(double value)
	{
		return TimeSpan(value / 1000.0);
	}

	std::string FormatTime()
	{
		double absoluteTime = abs(time);

		double minutes = floor(fmod(absoluteTime, 3600.0) / 60.0);
		double seconds = fmod(absoluteTime, 60.0);
		double milliseconds = (seconds - floor(seconds)) * 1000.0;

		char buffer[12]; // 00:00.000
		const char* sign = time < 0 ? "-" : "";
		sprintf_s(buffer, sizeof(buffer), "%s%02d:%02d.%03d", sign, (int)minutes, (int)seconds, (int)milliseconds);
	
		return std::string(buffer);
	}

	// Operators:
	// ----------
	inline bool operator== (TimeSpan other) { return TotalSeconds() == other.TotalSeconds(); }
	inline bool operator!= (TimeSpan other) { return TotalSeconds() != other.TotalSeconds(); }
	inline bool operator<= (TimeSpan other) { return TotalSeconds() <= other.TotalSeconds(); }
	inline bool operator>= (TimeSpan other) { return TotalSeconds() >= other.TotalSeconds(); }
	inline bool operator< (TimeSpan other) { return TotalSeconds() < other.TotalSeconds(); }
	inline bool operator> (TimeSpan other) { return TotalSeconds() > other.TotalSeconds(); }
	inline TimeSpan operator+ (TimeSpan other) { return FromSeconds(other.TotalSeconds() + TotalSeconds()); }
	inline TimeSpan operator- (TimeSpan other) { return FromSeconds(other.TotalSeconds() - TotalSeconds()); }
	inline TimeSpan& operator+= (const TimeSpan& other) { this->time += other.time; return *this; }
	inline TimeSpan& operator-= (const TimeSpan& other) { this->time -= other.time; return *this; }
	inline TimeSpan operator* (double other) { return FromSeconds(TotalSeconds() * other); }
	inline TimeSpan operator* (int other) { return FromSeconds(TotalSeconds() * other); }
	// ----------

private:
	// Time in Seconds
	// ---------------
	double time; 
};