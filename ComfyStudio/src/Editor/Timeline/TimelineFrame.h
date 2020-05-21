#pragma once

namespace Comfy::Studio::Editor
{
	// NOTE: Frame based timeline unit
	struct TimelineFrame
	{
		TimelineFrame() : frames(0.0f) {}
		TimelineFrame(float frame) : frames(frame) {}

		inline float Frames() const { return frames; }

		inline bool operator==(const TimelineFrame other) const { return frames == other.frames; }
		inline bool operator!=(const TimelineFrame other) const { return frames != other.frames; }
		inline bool operator<=(const TimelineFrame other) const { return frames <= other.frames; }
		inline bool operator>=(const TimelineFrame other) const { return frames >= other.frames; }
		inline bool operator<(const TimelineFrame other) const { return frames < other.frames; }
		inline bool operator>(const TimelineFrame other) const { return frames > other.frames; }
		inline TimelineFrame operator+(const TimelineFrame other) const { return TimelineFrame(frames + other.frames); }
		inline TimelineFrame operator-(const TimelineFrame other) const { return TimelineFrame(frames - other.frames); }

	private:
		float frames;
	};
}
