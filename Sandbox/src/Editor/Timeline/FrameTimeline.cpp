#include "FrameTimeline.h"
#include "Editor/Theme.h"

namespace Editor
{
	TimelineFrame FrameTimeline::GetCursorFrame() const
	{
		return GetTimelineFrame(GetCursorTime());
	}

	TimelineFrame FrameTimeline::GetGridFrame() const
	{
		return TimelineFrame(gridDivision);
	}

	TimelineFrame FrameTimeline::FloorToGrid(TimelineFrame frame) const
	{
		return glm::floor(frame.Frames());
	}

	TimelineFrame FrameTimeline::RoundToGrid(TimelineFrame frame) const
	{
		return glm::round(frame.Frames());
	}

	float FrameTimeline::GetTimelinePosition(TimeSpan time) const
	{
		time -= GetTimelineTime(loopStartFrame);
		return TimelineBase::GetTimelinePosition(time) + timelineContentMarginWidth;
	}

	float FrameTimeline::GetTimelinePosition(TimelineFrame frame) const
	{
		return GetTimelinePosition(GetTimelineTime(frame));
	}

	TimelineFrame FrameTimeline::GetTimelineFrame(TimeSpan time) const
	{
		return TimelineFrame(static_cast<float>(time.TotalSeconds() / (1.0f / frameRate)));
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
		position += TimelineBase::GetTimelinePosition(GetTimelineTime(loopStartFrame));
		return TimelineBase::GetTimelineTime(position - timelineContentMarginWidth);
	}

	TimelineFrame FrameTimeline::GetTimelineFrameAtMouseX() const
	{
		float mouseX = glm::max(infoColumnRegion.Max.x, ImGui::GetMousePos().x);
		return RoundToGrid(GetTimelineFrame(ScreenToTimelinePosition(mouseX)));
	}

	void FrameTimeline::OnDrawTimlineDivisors()
	{
		const int framesPerBar = 10;

		char barStrBuffer[16];
		
		const int startFrame = static_cast<int>(loopStartFrame.Frames());
		const int endFrame = static_cast<int>(loopEndFrame.Frames());
		const int frameStep = static_cast<int>(gridDivision);

		int divisions = 0;
		for (int frame = startFrame; frame <= endFrame; frame += frameStep)
		{
			bool isBar = (frame == startFrame) || (frame == endFrame) || (frame % framesPerBar == 0);

			float screenX = glm::round(GetTimelinePosition(TimelineFrame(static_cast<float>(frame))) - GetScrollX());
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