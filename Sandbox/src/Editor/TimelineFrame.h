#pragma once

namespace Editor
{
	// Frame based timeline unit
	// -------------------------
	struct TimelineFrame
	{
		// Constructors / Destructors:
		// ---------------------------
		TimelineFrame() : frames(0.0f) {};
		TimelineFrame(float frame) : frames(frame) {};
		// ---------------------------

		// Conversion Methods:
		// -------------------
		inline float Frames() const
		{
			return frames;
		}
		// -------------------

		// Operators:
		// ----------
		inline bool operator== (const TimelineFrame other) const { return frames == other.frames; }
		inline bool operator!= (const TimelineFrame other) const { return frames != other.frames; }
		inline bool operator<= (const TimelineFrame other) const { return frames <= other.frames; }
		inline bool operator>= (const TimelineFrame other) const { return frames >= other.frames; }
		inline bool operator< (const TimelineFrame other) const { return frames < other.frames; }
		inline bool operator> (const TimelineFrame other) const { return frames > other.frames; }
		inline TimelineFrame operator+ (const TimelineFrame other) const { return TimelineFrame(frames + other.frames); }
		inline TimelineFrame operator- (const TimelineFrame other) const { return TimelineFrame(frames - other.frames); }
		// ----------

	private:
		// Fields:
		// -------
		float frames;
	};
}