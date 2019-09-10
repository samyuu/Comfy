#include "KeyFrameRenderer.h"
#include "AetTimeline.h"

namespace Editor
{
	KeyFrameRenderer::KeyFrameRenderer()
	{
	}

	KeyFrameRenderer::~KeyFrameRenderer()
	{
	}

	void KeyFrameRenderer::DrawKeyFrames(const AetTimeline* timeline, const KeyFrameProperties& keyFramesProperties)
	{
		ImDrawList* windowDrawList = Gui::GetWindowDrawList();

		float scrollX = timeline->GetScrollX();
		vec2 timelineTL = glm::round(vec2(timeline->GetTimelineContentRegion().GetTL() - vec2(scrollX, 0.0f)));

		const float rowHeight = timeline->GetRowHeight();
		float y = (rowHeight / 2.0f);

		for (const auto& keyFrames : keyFramesProperties)
		{
			bool opacityKeyFrames = &keyFrames == &keyFramesProperties.KeyFrames[7];

			for (const auto& keyFrame : keyFrames)
			{
				TimelineFrame keyFrameFrame = keyFrames.size() == 1 ? timeline->GetLoopStartFrame() : keyFrame.Frame;
				float timelineX = glm::round(timeline->GetTimelinePosition(keyFrameFrame));

				TimelineVisibility visiblity = timeline->GetTimelineVisibility(timelineX - scrollX);
				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				vec2 position = vec2(timelineTL.x + timelineX, timelineTL.y + y);
				if (opacityKeyFrames)
					DrawSingleKeyFrame(windowDrawList, position, keyFrame.Value);
				else
					DrawSingleKeyFrame(windowDrawList, position);
			}

			y += rowHeight;
		}
	}

	void KeyFrameRenderer::DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position) const
	{
		ImU32 fillColor = GetColor(EditorColor_KeyFrame);
		ImU32 borderColor = GetColor(EditorColor_KeyFrameBorder);

		DrawSingleKeyFrame(drawList, position, fillColor, borderColor);
	}

	void KeyFrameRenderer::DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position, float opacity) const
	{
		ImU32 fillColor = GetColor(EditorColor_KeyFrame, opacity);
		ImU32 borderColor = GetColor(EditorColor_KeyFrameBorder);

		DrawSingleKeyFrame(drawList, position, fillColor, borderColor);
	}

	void KeyFrameRenderer::DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position, ImU32 fillColor, ImU32 borderColor) const
	{
		vec2 positions[3];
		positions[0] = position - vec2(keyFrameSize, 0.0f);
		positions[1] = position - vec2(0.0f, keyFrameSize);
		positions[2] = position + vec2(0.0f, keyFrameSize);

		drawList->AddTriangleFilled(positions[0], positions[1], positions[2], fillColor);
		drawList->AddLine(positions[0], positions[1], borderColor);
		drawList->AddLine(positions[2], positions[0], borderColor);

		positions[0] = position + vec2(keyFrameSize, 0.0f);
		positions[1] = position - vec2(0.0f, keyFrameSize);
		positions[2] = position + vec2(0.0f, keyFrameSize);

		drawList->AddTriangleFilled(positions[0], positions[1], positions[2], fillColor);
		drawList->AddLine(positions[0], positions[1], borderColor);
		drawList->AddLine(positions[2], positions[0], borderColor);
	}
}