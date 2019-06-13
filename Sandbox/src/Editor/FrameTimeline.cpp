#include "FrameTimeline.h"
#include "Theme.h"

namespace Editor
{
	TimelineFrame FrameTimeline::GetGridFrame() const
	{
		return TimelineFrame(gridDivision);
	}

	TimelineFrame FrameTimeline::FloorToGrid(TimelineFrame frame) const
	{
		// TODO: ...
		return floor(frame.Frames());

		float gridFrame = GetGridFrame().Frames();
		return floor(gridFrame);

		//float gridFrame = GetGridFrame().Frames();
		//return (floor((int)gridFrame / gridFrame) * gridFrame);
	}

	TimelineFrame FrameTimeline::RoundToGrid(TimelineFrame frame) const
	{
		float gridFrame = GetGridFrame().Frames();
		return (round((int)gridFrame / gridFrame) * gridFrame);
	}

	float FrameTimeline::GetTimelinePosition(TimeSpan time) const
	{
		return TimelineBase::GetTimelinePosition(time);
	}

	float FrameTimeline::GetTimelinePosition(TimelineFrame frame) const
	{
		return GetTimelinePosition(GetTimelineTime(frame));
	}

	TimelineFrame FrameTimeline::GetTimelineFrame(TimeSpan time) const
	{
		return time.TotalSeconds() / (1.0f / frameRate);
	}

	TimelineFrame FrameTimeline::GetTimelineFrame(float position) const
	{
		return GetTimelineFrame(GetTimelineTime(position));
	}

	TimeSpan FrameTimeline::GetTimelineTime(TimelineFrame frame) const
	{
		return (1.0f / frameRate) * frame.Frames();
	}

	TimeSpan FrameTimeline::GetTimelineTime(float position) const
	{
		return TimelineBase::GetTimelineTime(position);
	}

	TimelineFrame FrameTimeline::GetCursorFrame() const
	{
		return GetTimelineFrame(GetCursorTime());
	}

	TimelineFrame FrameTimeline::GetCursorMouseXFrame() const
	{
		return FloorToGrid(GetTimelineFrame(ScreenToTimelinePosition(ImGui::GetMousePos().x)));
	}

	void FrameTimeline::OnDrawTimlineDivisors()
	{
		const int framesPerBar = 30;

		char barStrBuffer[16];

		const int totalFrames = static_cast<int>(endFrame.Frames());
		const int frameStep = gridDivision;

		const float scrollX = GetScrollX();

		for (int frame = 0, divisions = 0; frame <= totalFrames; frame += frameStep)
		{
			bool isBar = frame % framesPerBar == 0;

			float screenX = GetTimelinePosition(TimelineFrame(frame)) - scrollX;
			TimelineVisibility visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const float startYOffset = timelineHeaderHeight * (isBar ? .85f : .35f);
			ImVec2 start = timelineContentRegion.GetTL() + ImVec2(screenX, -startYOffset);
			ImVec2 end = timelineContentRegion.GetBL() + ImVec2(screenX, 0);

			const ImU32 color = GetColor(isBar ? EditorColor_Bar : (divisions++ % 2 == 0 ? EditorColor_Grid : EditorColor_GridAlt));
			baseDrawList->AddLine(start, end, color);

			if (isBar)
			{
				sprintf_s(barStrBuffer, sizeof(barStrBuffer), "%d", frame);

				start += ImVec2(3, -1);
				baseDrawList->AddText(start, color, barStrBuffer);
			}
		}
	}

	void FrameTimeline::DrawTimelineCursor()
	{
		TimelineBase::DrawTimelineCursor();
	}
}