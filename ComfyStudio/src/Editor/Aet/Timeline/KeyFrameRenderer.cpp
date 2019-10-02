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

	void KeyFrameRenderer::DrawLayerObjects(const AetTimeline* timeline, const RefPtr<AetLayer>& layer, frame_t frame)
	{
		ImDrawList* windowDrawList = Gui::GetWindowDrawList();

		const float scrollX = timeline->GetScrollX();
		const vec2 timelineTL = glm::round(vec2(timeline->GetTimelineContentRegion().GetTL() - vec2(scrollX, 0.0f)));

		const float rowHeight = timeline->GetRowHeight();
		float y = (rowHeight / 2.0f) + 0.5f;

		for (size_t i = 0; i < layer->size(); i++)
		{
			// TODO: Add Y scroll offset
			const auto& object = layer->at(i);

			const float timelineStartX = glm::round(timeline->GetTimelinePosition(TimelineFrame(object->LoopStart)));
			const float timelineEndX = glm::round(timeline->GetTimelinePosition(TimelineFrame(object->LoopEnd)));

			const vec2 startPosition = vec2(timelineTL.x + timelineStartX, timelineTL.y + y);
			const vec2 endPosition = vec2(timelineTL.x + timelineEndX, startPosition.y);

			const bool isActive = (frame >= object->LoopStart) && (frame <= object->LoopEnd);

			// TODO: Implement culling
			DrawKeyFrameConnection(windowDrawList, startPosition, endPosition, isActive);
			DrawSingleKeyFrame(windowDrawList, startPosition);
			DrawSingleKeyFrame(windowDrawList, endPosition);

			y += rowHeight;
		}
	}

	void KeyFrameRenderer::DrawKeyFrames(const AetTimeline* timeline, const KeyFrameProperties& keyFramesProperties)
	{
		ImDrawList* windowDrawList = Gui::GetWindowDrawList();

		const float scrollX = timeline->GetScrollX();
		const vec2 timelineTL = glm::round(vec2(timeline->GetTimelineContentRegion().GetTL() - vec2(scrollX, 0.0f)));

		const float rowHeight = timeline->GetRowHeight();
		float y = (rowHeight / 2.0f) + 0.5f;

		for (const auto& keyFrames : keyFramesProperties)
		{
			const bool opacityKeyFrames = &keyFrames == &keyFramesProperties.KeyFrames[7];

			for (const auto& keyFrame : keyFrames)
			{
				const float timelineX = glm::round(timeline->GetTimelinePosition(TimelineFrame(keyFrame.Frame))) + 1.0f;

				TimelineVisibility visiblity = timeline->GetTimelineVisibility(timelineX - scrollX);
				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				const vec2 position = vec2(timelineTL.x + timelineX, timelineTL.y + y);
				if (opacityKeyFrames)
					DrawSingleKeyFrame(windowDrawList, position, keyFrame.Value);
				else
					DrawSingleKeyFrame(windowDrawList, position);
			}

			y += rowHeight;
		}
	}

	void KeyFrameRenderer::DrawKeyFrameConnection(ImDrawList* drawList, const vec2& start, const vec2& end, bool active) const
	{
		vec2 topLeft = start - vec2(0.0f, keyFrameSize);
		vec2 topRight = end - vec2(0.0f, keyFrameSize);
		vec2 bottomLeft = start + vec2(0.0f, keyFrameSize);
		vec2 bottomRight = end + vec2(0.0f, keyFrameSize);

		// TODO: Rethink this (?)
		float alpha = active ? 0.7f : 1.0f;
		ImU32 fillColor = GetColor(EditorColor_KeyFrameConnection, alpha);
		ImU32 fillColorAlt = GetColor(EditorColor_KeyFrameConnectionAlt, alpha);
		ImU32 borderColor = GetColor(EditorColor_KeyFrameBorder);

		drawList->AddRectFilledMultiColor(topLeft, bottomRight, fillColor, fillColor, fillColorAlt, fillColorAlt);

		drawList->AddLine(topLeft, topRight, borderColor, 1.0f);
		drawList->AddLine(bottomLeft, bottomRight, borderColor, 1.0f);
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
		// TODO: Use textures to ensure pixel perfection

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