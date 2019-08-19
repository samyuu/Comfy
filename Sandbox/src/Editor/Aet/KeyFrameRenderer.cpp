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
		vec2 timelineTL = timeline->GetTimelineContentRegion().GetTL() - vec2(timeline->GetScrollX(), 0.0f);

		const float rowHeight = timeline->GetRowHeight();
		float y = (rowHeight / 2.0f);

		for (const auto& keyFrames : keyFramesProperties)
		{
			for (const auto& keyFrame : keyFrames)
			{
				TimelineFrame keyFrameFrame = keyFrames.size() == 1 ? timeline->GetLoopStartFrame() : keyFrame.Frame;

				vec2 position = vec2(glm::round(timelineTL.x + timeline->GetTimelinePosition(keyFrameFrame)), timelineTL.y + y);
				DrawSingleKeyFrame(windowDrawList, position);
			}

			y += rowHeight;
		}
	}

	void KeyFrameRenderer::DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position) const
	{
		vec2 positions[3];
		positions[0] = position - vec2(keyFrameSize, 0.0f);
		positions[1] = position - vec2(0.0f, keyFrameSize);
		positions[2] = position + vec2(0.0f, keyFrameSize);

		drawList->AddTriangleFilled(positions[0], positions[1], positions[2], GetColor(EditorColor_KeyFrame));
		drawList->AddLine(positions[0], positions[1], GetColor(EditorColor_KeyFrameBorder));
		drawList->AddLine(positions[2], positions[0], GetColor(EditorColor_KeyFrameBorder));

		positions[0] = position + vec2(keyFrameSize, 0.0f);
		positions[1] = position - vec2(0.0f, keyFrameSize);
		positions[2] = position + vec2(0.0f, keyFrameSize);

		drawList->AddTriangleFilled(positions[0], positions[1], positions[2], GetColor(EditorColor_KeyFrame));
		drawList->AddLine(positions[0], positions[1], GetColor(EditorColor_KeyFrameBorder));
		drawList->AddLine(positions[2], positions[0], GetColor(EditorColor_KeyFrameBorder));
	}
}