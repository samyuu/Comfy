#pragma once
#include "TimelineBase.h"
#include "TimelineFrame.h"

namespace Editor
{
	class FrameTimeline : public TimelineBase
	{
	public:
		TimelineFrame GetCursorFrame() const;

		TimelineFrame GetGridFrame() const;
		TimelineFrame FloorToGrid(TimelineFrame frame) const;
		TimelineFrame RoundToGrid(TimelineFrame frame) const;

		float GetTimelinePosition(TimeSpan time) const override;
		float GetTimelinePosition(TimelineFrame frame) const;

		TimelineFrame GetTimelineFrame(TimeSpan time) const;
		TimelineFrame GetTimelineFrame(float position) const;

		TimeSpan GetTimelineTime(TimelineFrame frame) const;
		TimeSpan GetTimelineTime(float position) const override;

		TimelineFrame GetTimelineFrameAtMouseX() const;

		inline TimelineFrame GetLoopStartFrame() const { return loopStartFrame; };
		inline TimelineFrame GetLoopEndFrame() const { return loopEndFrame; };

	protected:
		const float timelineContentMarginWidth = 40.0f;

		TimelineFrame loopStartFrame = 0.0f;
		TimelineFrame loopEndFrame = 60.0f;
		float frameRate = 60.0f;
		float gridDivision = 1.0f;

		void OnDrawTimlineDivisors() override;
		void DrawTimelineCursor() override;
	};
}