#pragma once
#include "TimelineBase.h"
#include "TimelineFrame.h"

namespace Comfy::Studio::Editor
{
	class FrameTimeline : public TimelineBase
	{
	public:
		TimelineFrame GetCursorFrame() const;

		TimelineFrame GetGridFrame() const;
		TimelineFrame FloorToGrid(TimelineFrame frame) const;
		TimelineFrame RoundToGrid(TimelineFrame frame) const;

		f32 GetTimelinePosition(TimeSpan time) const override;
		f32 GetTimelinePosition(TimelineFrame frame) const;

		TimelineFrame GetTimelineFrame(TimeSpan time) const;
		TimelineFrame GetTimelineFrame(f32 position) const;

		TimeSpan GetTimelineTime(TimelineFrame frame) const;
		TimeSpan GetTimelineTime(f32 position) const override;

		TimelineFrame GetTimelineFrameAtMouseX() const;

		inline TimelineFrame GetLoopStartFrame() const { return loopStartFrame; }
		inline TimelineFrame GetLoopEndFrame() const { return loopEndFrame; }

	protected:
		void OnDrawTimlineDivisors() override;
		void DrawTimelineCursor() override;

	protected:
		const f32 timelineContentMarginWidth = 40.0f;

		TimelineFrame loopStartFrame = 0.0f;
		TimelineFrame loopEndFrame = 60.0f;
		f32 frameRate = 60.0f;
		f32 gridDivision = 1.0f;
	};
}
