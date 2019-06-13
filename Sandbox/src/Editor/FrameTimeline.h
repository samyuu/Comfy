#pragma once
#include "TimelineBase.h"
#include "TimelineFrame.h"

namespace Editor
{
	class FrameTimeline : public TimelineBase
	{
	protected:
		// TODO: offset everything by start frame
		TimelineFrame endFrame = 60.0f;
		float frameRate = 60.0f;

		// const std::array<const char*, 10> gridDivisionStrings = { "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
		// const std::array<int, 10> gridDivisions = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64 };
		// int gridDivisionIndex = 0;

		float gridDivision = 1.0f;

		// Conversion Methods:
		// -------------------
		TimelineFrame GetGridFrame() const;
		TimelineFrame FloorToGrid(TimelineFrame frame) const;
		TimelineFrame RoundToGrid(TimelineFrame frame) const;

		virtual float GetTimelinePosition(TimeSpan time) const override;
		float GetTimelinePosition(TimelineFrame frame) const;

		TimelineFrame GetTimelineFrame(TimeSpan time) const;
		TimelineFrame GetTimelineFrame(float position) const;

		TimeSpan GetTimelineTime(TimelineFrame frame) const;
		TimeSpan GetTimelineTime(float position) const override;

		TimelineFrame GetCursorFrame() const;
		TimelineFrame GetCursorMouseXFrame() const;
		// -------------------

		void OnDrawTimlineDivisors() override;
		void DrawTimelineCursor() override;
	};
}