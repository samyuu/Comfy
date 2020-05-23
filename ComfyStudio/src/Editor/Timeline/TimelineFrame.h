#pragma once

namespace Comfy::Studio::Editor
{
	// NOTE: Frame based timeline unit
	struct TimelineFrame
	{
	public:
		constexpr TimelineFrame() : frames(0.0f) {}
		constexpr TimelineFrame(float frame) : frames(frame) {}

	public:
		constexpr float Frames() const { return frames; }

		constexpr bool operator==(const TimelineFrame other) const { return frames == other.frames; }
		constexpr bool operator!=(const TimelineFrame other) const { return frames != other.frames; }
		constexpr bool operator<=(const TimelineFrame other) const { return frames <= other.frames; }
		constexpr bool operator>=(const TimelineFrame other) const { return frames >= other.frames; }
		constexpr bool operator<(const TimelineFrame other) const { return frames < other.frames; }
		constexpr bool operator>(const TimelineFrame other) const { return frames > other.frames; }
		constexpr TimelineFrame operator+(const TimelineFrame other) const { return TimelineFrame(frames + other.frames); }
		constexpr TimelineFrame operator-(const TimelineFrame other) const { return TimelineFrame(frames - other.frames); }

	private:
		float frames;
	};
}
