#include "FrameTimeline.h"
#include "Editor/Core/Theme.h"

namespace Comfy::Studio::Editor
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

	f32 FrameTimeline::GetTimelinePosition(TimeSpan time) const
	{
		time -= GetTimelineTime(loopStartFrame);
		return TimelineBase::GetTimelinePosition(time) + timelineContentMarginWidth;
	}

	f32 FrameTimeline::GetTimelinePosition(TimelineFrame frame) const
	{
		return GetTimelinePosition(GetTimelineTime(frame));
	}

	TimelineFrame FrameTimeline::GetTimelineFrame(TimeSpan time) const
	{
		return TimelineFrame(static_cast<f32>(time.TotalSeconds() / (1.0 / frameRate)));
	}

	TimelineFrame FrameTimeline::GetTimelineFrame(f32 position) const
	{
		return GetTimelineFrame(GetTimelineTime(position));
	}

	TimeSpan FrameTimeline::GetTimelineTime(TimelineFrame frame) const
	{
		return TimeSpan::FromSeconds((1.0 / frameRate) * frame.Frames());
	}

	TimeSpan FrameTimeline::GetTimelineTime(f32 position) const
	{
		position += TimelineBase::GetTimelinePosition(GetTimelineTime(loopStartFrame));
		return TimelineBase::GetTimelineTime(position - timelineContentMarginWidth);
	}

	TimelineFrame FrameTimeline::GetTimelineFrameAtMouseX() const
	{
		const f32 mouseX = glm::max(regions.InfoColumnContent.Max.x, Gui::GetMousePos().x);
		return RoundToGrid(GetTimelineFrame(ScreenToTimelinePosition(mouseX)));
	}

	void FrameTimeline::OnDrawTimlineDivisors()
	{
		constexpr i32 framesPerBar = 10;

		const i32 startFrame = static_cast<i32>(loopStartFrame.Frames());
		const i32 endFrame = static_cast<i32>(loopEndFrame.Frames());
		const i32 frameStep = static_cast<i32>(gridDivision);

		i32 divisions = 0;
		for (i32 frame = startFrame; frame <= endFrame; frame += frameStep)
		{
			const bool isBar = (frame == startFrame) || (frame == endFrame) || (frame % framesPerBar == 0);

			const f32 screenX = glm::round(GetTimelinePosition(TimelineFrame(static_cast<f32>(frame))) - GetScrollX());
			TimelineVisibility visiblity = GetTimelineVisibility(screenX);

			if (visiblity == TimelineVisibility::Left)
				continue;
			if (visiblity == TimelineVisibility::Right)
				break;

			const f32 startYOffset = timelineHeaderHeight * (isBar ? 0.85f : 0.35f);
			vec2 start = regions.Content.GetTL() + vec2(screenX, -startYOffset);
			vec2 end = regions.Content.GetBL() + vec2(screenX, 0);

			const ImU32 color = GetColor(isBar ? EditorColor_Bar : (divisions++ % 2 == 0 ? EditorColor_Grid : EditorColor_GridAlt));
			baseDrawList->AddLine(start, end, color);

			if (isBar)
			{
				char barStringBuffer[16];
				sprintf_s(barStringBuffer, sizeof(barStringBuffer), "%d", frame);

				start += vec2(3.0f, -1.0f);
				baseDrawList->AddText(start, color, barStringBuffer);
			}
		}
	}

	void FrameTimeline::DrawTimelineCursor()
	{
		TimelineBase::DrawTimelineCursor();
	}
}
