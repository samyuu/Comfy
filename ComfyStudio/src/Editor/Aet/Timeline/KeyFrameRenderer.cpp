#include "KeyFrameRenderer.h"
#include "AetTimeline.h"

namespace Editor
{
#define _ 0x00000000
#define X 0xFFFFFFFF
	const uint32_t KeyFrameRenderer::keyFrameTexturePixels[keyFrameTextureSize.x * keyFrameTextureSize.y] = 
	{
		////////////////////////////////////////////////////////
		/**/_,_,_,_,_,X,_,_,_,_,_,/**/_,_,_,_,_,_,_,_,_,_,_,/**/
		/**/_,_,_,_,X,_,X,_,_,_,_,/**/_,X,X,X,X,X,X,X,X,X,_,/**/
		/**/_,_,_,X,_,_,_,X,_,_,_,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/_,_,X,_,_,_,_,_,X,_,_,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/_,X,_,_,_,_,_,_,_,X,_,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/X,_,_,_,_,_,_,_,_,_,X,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/_,X,_,_,_,_,_,_,_,X,_,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/_,_,X,_,_,_,_,_,X,_,_,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/_,_,_,X,_,_,_,X,_,_,_,/**/_,X,_,_,_,_,_,_,_,X,_,/**/
		/**/_,_,_,_,X,_,X,_,_,_,_,/**/_,X,X,X,X,X,X,X,X,X,_,/**/
		/**/_,_,_,_,_,X,_,_,_,_,_,/**/_,_,_,_,_,_,_,_,_,_,_,/**/
		////------------------------------------------------////
		/**/_,_,_,_,_,_,_,_,_,_,_,/**/_,_,_,_,_,_,_,_,_,_,_,/**/
		/**/_,_,_,_,_,X,_,_,_,_,_,/**/_,_,_,_,_,_,_,_,_,_,_,/**/
		/**/_,_,_,_,X,X,X,_,_,_,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,_,_,X,X,X,X,X,_,_,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,_,X,X,X,X,X,X,X,_,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,X,X,X,X,X,X,X,X,X,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,_,X,X,X,X,X,X,X,_,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,_,_,X,X,X,X,X,_,_,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,_,_,_,X,X,X,_,_,_,_,/**/_,_,X,X,X,X,X,X,X,_,_,/**/
		/**/_,_,_,_,_,X,_,_,_,_,_,/**/_,_,_,_,_,_,_,_,_,_,_,/**/
		/**/_,_,_,_,_,_,_,_,_,_,_,/**/_,_,_,_,_,_,_,_,_,_,_,/**/
		////////////////////////////////////////////////////////
	};
#undef X
#undef _

	KeyFrameRenderer::KeyFrameRenderer()
	{
	}

	KeyFrameRenderer::~KeyFrameRenderer()
	{
	}

	void KeyFrameRenderer::Initialize()
	{
		CreateKeyFrameTexture();
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

			// TODO: Make sure to draw all connection lines before drawing any keyframes so they can be batched together correctly
			// TODO: Implement culling
			DrawKeyFrameConnection(windowDrawList, startPosition, endPosition, isActive);
			DrawSingleKeyFrame(windowDrawList, startPosition, KeyFrameType::InBetween); // KeyFrameType::First
			DrawSingleKeyFrame(windowDrawList, endPosition, KeyFrameType::InBetween); // KeyFrameType::Last

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
				const float timelineX = glm::round(timeline->GetTimelinePosition(TimelineFrame(keyFrame.Frame)));

				TimelineVisibility visiblity = timeline->GetTimelineVisibility(timelineX - scrollX);
				if (visiblity == TimelineVisibility::Left)
					continue;
				if (visiblity == TimelineVisibility::Right)
					break;

				const vec2 position = vec2(timelineTL.x + timelineX, timelineTL.y + y);
				const KeyFrameType type = GetKeyFrameType(keyFrame, keyFrames);
				const float opacity = opacityKeyFrames ? keyFrame.Value : 1.0f;

				DrawSingleKeyFrame(windowDrawList, position, type, opacity);
			}

			y += rowHeight;
		}
	}

	void KeyFrameRenderer::CreateKeyFrameTexture()
	{
		keyFrameTexture = MakeUnique<Graphics::Texture2D>();
		keyFrameTexture->CreateFromRgbaBuffer(keyFrameTextureSize, keyFrameTexturePixels);
	}

	void KeyFrameRenderer::DrawKeyFrameConnection(ImDrawList* drawList, const vec2& start, const vec2& end, bool active) const
	{
		vec2 topLeft = start - vec2(0.0f, keyFrameSize);
		vec2 topRight = end - vec2(0.0f, keyFrameSize);
		vec2 bottomLeft = start + vec2(0.0f, keyFrameSize - 1.0f);
		vec2 bottomRight = end + vec2(0.0f, keyFrameSize - 1.0f);

		// TODO: Rethink this (?)
		float alpha = active ? 0.7f : 1.0f;
		ImU32 fillColor = GetColor(EditorColor_KeyFrameConnection, alpha);
		ImU32 fillColorAlt = GetColor(EditorColor_KeyFrameConnectionAlt, alpha);
		ImU32 borderColor = GetColor(EditorColor_KeyFrameBorder);

		drawList->AddRectFilledMultiColor(topLeft, bottomRight, fillColor, fillColor, fillColorAlt, fillColorAlt);

		drawList->AddLine(topLeft, topRight, borderColor, 1.0f);
		drawList->AddLine(bottomLeft, bottomRight, borderColor, 1.0f);
	}

	void KeyFrameRenderer::DrawKeyFramePart(ImDrawList* drawList, vec2 position, KeyFramePart type, ImU32 color) const
	{
		static_assert(keyFrameTextureSize == ivec2(22.0, 22.0));

		vec2 topLeft = position - vec2(5.0f, 5.5f);
		vec4 sourceRegion;

		switch (type)
		{
		case KeyFramePart::Border:
			sourceRegion = vec4(0.0f, 0.0f, 11.0f, 11.0f);
			break;
		case KeyFramePart::FillFull:
			sourceRegion = vec4(0.0f, 11.0f, 11.0f, 11.0f);
			break;
		case KeyFramePart::FillLeft:
			sourceRegion = vec4(0.0f, 11.0f, 6.0f, 11.0f);
			break;
		case KeyFramePart::FillRight:
			sourceRegion = vec4(5.0f, 11.0f, 6.0f, 11.0f);
			topLeft += vec2(5.0f, 0.0f);
			break;
		case KeyFramePart::SquareBorder:
			sourceRegion = vec4(11.0f, 0.0f, 11.0f, 11.0f);
			break;
		case KeyFramePart::Square:
			sourceRegion = vec4(11.0f, 11.0f, 11.0f, 11.0f);
			break;
		default:
			return;
		}

		Gui::AddSprite(drawList, keyFrameTexture.get(), topLeft, sourceRegion, color);
	}

	void KeyFrameRenderer::DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position, KeyFrameType type, float opacity) const
	{
		const ImU32 fillColor = GetColor(EditorColor_KeyFrame, opacity);
		const ImU32 fillColorHalf = GetColor(EditorColor_KeyFrameBorder, 0.5f * opacity);
		const ImU32 borderColor = GetColor(EditorColor_KeyFrameBorder);

		switch (type)
		{
		case KeyFrameType::Single:
			DrawKeyFramePart(drawList, position, KeyFramePart::Square, fillColor);
			DrawKeyFramePart(drawList, position, KeyFramePart::SquareBorder, borderColor);
			break;
		case KeyFrameType::First:
			DrawKeyFramePart(drawList, position, KeyFramePart::FillFull, fillColor);
			DrawKeyFramePart(drawList, position, KeyFramePart::FillLeft, fillColorHalf);
			DrawKeyFramePart(drawList, position, KeyFramePart::Border, borderColor);
			break;
		case KeyFrameType::Last:
			DrawKeyFramePart(drawList, position, KeyFramePart::FillFull, fillColor);
			DrawKeyFramePart(drawList, position, KeyFramePart::FillRight, fillColorHalf);
			DrawKeyFramePart(drawList, position, KeyFramePart::Border, borderColor);
			break;
		case KeyFrameType::InBetween:
			DrawKeyFramePart(drawList, position, KeyFramePart::FillFull, fillColor);
			DrawKeyFramePart(drawList, position, KeyFramePart::Border, borderColor);
			break;
		}
	}

	KeyFrameRenderer::KeyFrameType KeyFrameRenderer::GetKeyFrameType(const AetKeyFrame& keyFrame, const KeyFrameCollection& keyFrames)
	{
		if (keyFrames.size() == 1)
		{
			return KeyFrameType::Single;
		}
		else if (&keyFrame == &keyFrames.front())
		{
			return KeyFrameType::First;
		}
		else if (&keyFrame == &keyFrames.back())
		{
			return KeyFrameType::Last;
		}
		else
		{
			return KeyFrameType::InBetween;
		}
	}
}