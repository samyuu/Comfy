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
		double minutes = floor(fmod(time, 3600.0) / 60.0);
		double seconds = fmod(time, 60.0);
		double milliseconds = (seconds - floor(seconds)) * 1000.0;

		char buffer[12]; // 00:00.000
		sprintf_s(buffer, sizeof(buffer), "%02d:%02d.%03d", (int)minutes, (int)seconds, (int)milliseconds);
	
		return std::string(buffer);
	}

private:
	// Time in Seconds
	// ---------------
	double time; 
};