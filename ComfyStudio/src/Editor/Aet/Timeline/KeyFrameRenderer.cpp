#include "KeyFrameRenderer.h"
#include "AetTimeline.h"
#include "Graphics/TexSet.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

#define _ 0x00000000
#define X 0xFFFFFFFF
	constexpr ivec2 AetKeyFrameTextureSize = ivec2(22, 22);
	const std::array<u32, (AetKeyFrameTextureSize.x * AetKeyFrameTextureSize.y)> AetKeyFrameTexturePixels =
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
		CreateKeyFrameTexture();
	}

	void KeyFrameRenderer::DrawContent(const AetTimeline& timeline, const Aet::Composition* workingComp)
	{
		assert(workingComp != nullptr);

		ImDrawList* drawList = Gui::GetWindowDrawList();
		const frame_t cursorFrame = timeline.GetCursorFrame().Frames();

		i32 currentRow = 0;
		for (auto& layer : workingComp->GetLayers())
		{
			const vec2 startPosition = GetCenteredTimelineRowScreenPosition(timeline, layer->StartFrame, currentRow);
			const vec2 endPosition = GetCenteredTimelineRowScreenPosition(timeline, layer->EndFrame, currentRow);
			++currentRow;

			const bool isActive = (cursorFrame >= layer->StartFrame) && (cursorFrame <= layer->EndFrame);
			DrawKeyFrameConnection(drawList, startPosition, endPosition, isActive);

			DrawSingleKeyFrame(drawList, startPosition, KeyFrameType::InBetween);
			DrawSingleKeyFrame(drawList, endPosition, KeyFrameType::InBetween);

			if (!layer->GuiData.TimelineNodeOpen)
				continue;

			// TODO: The same should happen for the info column
			if (layer->LayerVideo == nullptr)
				continue;

			for (Transform2DField i = 0; i < Transform2DField_Count; i++)
			{
				auto& property = layer->LayerVideo->Transform[i];
				for (const auto& keyFrame : property.Keys)
				{
					const vec2 keyFramePosition = GetCenteredTimelineRowScreenPosition(timeline, keyFrame.Frame, currentRow);
					const TimelineVisibility visiblity = timeline.GetTimelineVisibilityForScreenSpace(keyFramePosition.x);

					if (visiblity == TimelineVisibility::Visible)
					{
						const bool isOpacity = (i == Transform2DField_Opacity);
						DrawSingleKeyFrame(drawList, keyFramePosition, GetKeyFrameType(keyFrame, property), GetKeyFrameOpacity(keyFrame, isOpacity));
					}
				}
				++currentRow;
			}
		}
	}

	vec2 KeyFrameRenderer::GetCenteredTimelineRowScreenPosition(const AetTimeline& timeline, frame_t frame, i32 row)
	{
		constexpr f32 pixelPerfectOffset = 0.5f;

		const f32 rowHeight = timeline.GetRowItemHeight();
		const f32 halfRowHeight = rowHeight / 2.0f;

		const f32 xWorldSpace = glm::round(timeline.GetTimelinePosition(TimelineFrame(frame)));
		const f32 yWorldSpace = (rowHeight * row) + halfRowHeight + pixelPerfectOffset;

		const vec2 scrollOffset = vec2(timeline.GetScrollX(), timeline.GetScrollY());
		const vec2 timelineTL = glm::round(vec2(timeline.GetRegions().Content.GetTL() - scrollOffset));

		return vec2(timelineTL.x + xWorldSpace, timelineTL.y + yWorldSpace);
	}

	void KeyFrameRenderer::CreateKeyFrameTexture()
	{
		keyFrameTexture = std::make_unique<Tex>();
		auto& baseMipMap = keyFrameTexture->MipMapsArray.emplace_back().emplace_back();

		baseMipMap.Size = AetKeyFrameTextureSize;
		baseMipMap.Format = TextureFormat::RGBA8;
		baseMipMap.DataSize = sizeof(AetKeyFrameTexturePixels);
		baseMipMap.Data = std::make_unique<u8[]>(baseMipMap.DataSize);
		std::memcpy(baseMipMap.Data.get(), AetKeyFrameTexturePixels.data(), baseMipMap.DataSize);
	}

	void KeyFrameRenderer::DrawKeyFrameConnection(ImDrawList* drawList, const vec2& start, const vec2& end, bool active) const
	{
		vec2 topLeft = start - vec2(0.0f, keyFrameSize);
		vec2 topRight = end - vec2(0.0f, keyFrameSize);
		vec2 bottomLeft = start + vec2(0.0f, keyFrameSize - 1.0f);
		vec2 bottomRight = end + vec2(0.0f, keyFrameSize - 1.0f);

		// TODO: Rethink this (?)
		const f32 alpha = active ? 0.7f : 1.0f;
		ImU32 fillColor = GetColor(EditorColor_KeyFrameConnection, alpha);
		ImU32 fillColorAlt = GetColor(EditorColor_KeyFrameConnectionAlt, alpha);
		ImU32 borderColor = GetColor(EditorColor_KeyFrameBorder);

		drawList->AddRectFilledMultiColor(topLeft, bottomRight, fillColor, fillColor, fillColorAlt, fillColorAlt);

		drawList->AddLine(topLeft, topRight, borderColor, 1.0f);
		drawList->AddLine(bottomLeft, bottomRight, borderColor, 1.0f);
	}

	void KeyFrameRenderer::DrawKeyFramePart(ImDrawList* drawList, vec2 position, KeyFramePart type, ImU32 color) const
	{
		static_assert(AetKeyFrameTextureSize == ivec2(22.0, 22.0));

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

	void KeyFrameRenderer::DrawSingleKeyFrame(ImDrawList* drawList, const vec2& position, KeyFrameType type, f32 opacity) const
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

	KeyFrameRenderer::KeyFrameType KeyFrameRenderer::GetKeyFrameType(const Aet::KeyFrame& keyFrame, const Aet::Property1D& property)
	{
		if (property->size() == 1)
		{
			return KeyFrameType::Single;
		}
		else if (&keyFrame == &property->front())
		{
			return KeyFrameType::First;
		}
		else if (&keyFrame == &property->back())
		{
			return KeyFrameType::Last;
		}
		else
		{
			return KeyFrameType::InBetween;
		}
	}

	f32 KeyFrameRenderer::GetKeyFrameOpacity(const Aet::KeyFrame& keyFrame, bool opactiyKeyFrames)
	{
		return opactiyKeyFrames ? keyFrame.Value : 1.0f;
	}
}
