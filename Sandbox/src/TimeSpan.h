#pragma once

// Time struct storing the underlying Time as a double in Seconds
// --------------------------------------------------------------
struct TimeSpan
{
	// Constructos:
	// ------------
	inline TimeSpan() : time(0) {}
	inline TimeSpan(double seconds) : time(seconds) {}
	// ------------

	inline double Minutes()
	{
		return Seconds() / 60.0;
	}

	inline double Seconds()
	{
		return time;
	}

	inline double Milliseconds()
	{
		return Seconds() * 1000.0;
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

private:
	// Time in Seconds
	// ---------------
	double time; 
};