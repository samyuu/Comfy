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

	inline double TotalMinutes() const
	{
		return TotalSeconds() / 60.0;
	}

	inline double TotalSeconds() const
	{
		return time;
	}

	inline double TotalMilliseconds() const
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

	std::string FormatTime() const
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
	inline bool operator== (const TimeSpan& other) const { return time == other.time; }
	inline bool operator!= (const TimeSpan& other) const { return time != other.time; }
	inline bool operator<= (const TimeSpan& other) const { return time <= other.time; }
	inline bool operator>= (const TimeSpan& other) const { return time >= other.time; }
	inline bool operator< (const TimeSpan& other) const { return time < other.time; }
	inline bool operator> (const TimeSpan& other) const { return time > other.time; }
	inline TimeSpan operator+ (const TimeSpan other) const { return FromSeconds(TotalSeconds() + other.TotalSeconds()); }
	inline TimeSpan operator- (const TimeSpan other) const { return FromSeconds(TotalSeconds() - other.TotalSeconds()); }
	inline TimeSpan& operator+= (const TimeSpan& other) { this->time += other.time; return *this; }
	inline TimeSpan& operator-= (const TimeSpan& other) { this->time -= other.time; return *this; }
	inline TimeSpan operator* (double other) { return FromSeconds(TotalSeconds() * other); }
	inline TimeSpan operator* (int other) { return FromSeconds(TotalSeconds() * other); }
	inline double operator/ (TimeSpan other) { return TotalSeconds() / other.TotalSeconds(); }
	inline double operator/ (double other) { return TotalSeconds() / other; }
	inline double operator/ (int other) { return TotalSeconds() / other; }
	inline TimeSpan operator- () const { return -time; }
	// ----------

	static TimeSpan GetTimeNow();
	// ---------------

private:
	// Time in Seconds
	// ---------------
	double time; 
};

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define uniquename(prefix) CONCAT(prefix, __COUNTER__)
#define DEBUG_STOPWATCH(description) DebugStopwatch uniquename(__DEBUG_STOPWATCH)(description)

struct DebugStopwatch
{
	DebugStopwatch(const char* description);
	~DebugStopwatch();

	const char* Description;
	TimeSpan TimeOnStart;
};