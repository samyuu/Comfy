#include "TimelineRenderHelper.h"
#include "System/ComfyData.h"
#include "ImGui/Extensions/ImGuiExtensions.h"

namespace Comfy::Studio::Editor
{
	void TimelineRenderHelper::OnEditorSpritesLoaded(const Graphics::SprSet* sprSet)
	{
		editorSprites = sprSet;
		if (editorSprites == nullptr)
			return;

		auto findSprite = [this](std::string_view spriteName)
		{
			const auto found = FindIfOrNull(editorSprites->Sprites, [&](const auto& spr) { return spr.Name == spriteName; });
			return (found != nullptr && InBounds(found->TextureIndex, editorSprites->TexSet.Textures)) ? found : nullptr;
		};

		sprites.ButtonIcons[static_cast<size_t>(ButtonType::Triangle)] = findSprite("TIMELINE_TRIANGLE");
		sprites.ButtonIcons[static_cast<size_t>(ButtonType::Square)] = findSprite("TIMELINE_SQUARE");
		sprites.ButtonIcons[static_cast<size_t>(ButtonType::Cross)] = findSprite("TIMELINE_CROSS");
		sprites.ButtonIcons[static_cast<size_t>(ButtonType::Circle)] = findSprite("TIMELINE_CIRCLE");
		sprites.ButtonIcons[static_cast<size_t>(ButtonType::SlideL)] = findSprite("TIMELINE_SLIDE_L");
		sprites.ButtonIcons[static_cast<size_t>(ButtonType::SlideR)] = findSprite("TIMELINE_SLIDE_R");

		sprites.ButtonIconsSync[static_cast<size_t>(ButtonType::Triangle)] = findSprite("TIMELINE_TRIANGLE_SYNC");
		sprites.ButtonIconsSync[static_cast<size_t>(ButtonType::Square)] = findSprite("TIMELINE_SQUARE_SYNC");
		sprites.ButtonIconsSync[static_cast<size_t>(ButtonType::Cross)] = findSprite("TIMELINE_CROSS_SYNC");
		sprites.ButtonIconsSync[static_cast<size_t>(ButtonType::Circle)] = findSprite("TIMELINE_CIRCLE_SYNC");
		sprites.ButtonIconsSync[static_cast<size_t>(ButtonType::SlideL)] = findSprite("TIMELINE_SLIDE_L_SYNC");
		sprites.ButtonIconsSync[static_cast<size_t>(ButtonType::SlideR)] = findSprite("TIMELINE_SLIDE_R_SYNC");

		sprites.ButtonIconsFrag[static_cast<size_t>(ButtonType::SlideL)] = findSprite("TIMELINE_SLIDE_CHAIN_L");
		sprites.ButtonIconsFrag[static_cast<size_t>(ButtonType::SlideR)] = findSprite("TIMELINE_SLIDE_CHAIN_R");
		sprites.ButtonIconsFragSync[static_cast<size_t>(ButtonType::SlideL)] = findSprite("TIMELINE_SLIDE_CHAIN_L_SYNC");
		sprites.ButtonIconsFragSync[static_cast<size_t>(ButtonType::SlideR)] = findSprite("TIMELINE_SLIDE_CHAIN_R_SYNC");

		sprites.HoldText = findSprite("TIMELINE_HOLD_TEXT");
		sprites.HoldTextSync = findSprite("TIMELINE_HOLD_TEXT_SYNC");
	}

	void TimelineRenderHelper::DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, f32 scale, f32 transparency) const
	{
		if (target.Flags.SameTypeSyncCount > 1)
		{
			scale *= sameTypeSyncStackScale;
			position += (target.Flags.SameTypeSyncIndex == 0) ? +sameTypeSyncStackOffset : -sameTypeSyncStackOffset;
		}

		const auto radius = buttonIconWidth * scale;

		const auto topLeft = position - (radius * 0.5f);
		const auto bottomRight = position + (radius * 0.5f);

		const auto color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF * transparency);

		if (const auto buttonSpr = GetButtonSpriteForTarget(target); buttonSpr != nullptr)
			Gui::AddSprite(drawList, *editorSprites, *buttonSpr, topLeft, bottomRight, color);
		else
			drawList->AddRect(position - vec2(radius * 0.25f), position + vec2(radius * 0.25f), color);

		if (target.Flags.IsHold)
		{
			if (const auto holdTextSpr = GetHoldTextSprite(target.Flags.IsSync); holdTextSpr != nullptr)
				Gui::AddSprite(drawList, *editorSprites, *holdTextSpr, topLeft, bottomRight, color);
			else
				drawList->AddText(Gui::GetFont(), 12.0f, position + vec2(-12.0f, 0.0f), color, "HOLD");
		}

#if COMFY_DEBUG && 0 // DEBUG: CHAIN FLAG TEST
		if (target.Flags.IsChainStart)
			drawList->AddCircle(position, 14.0f, 0xFFFFFF00);
		if (target.Flags.IsChainEnd)
			drawList->AddCircle(position, 9.0f, 0xFFFF00FF);
#endif

#if COMFY_DEBUG && 0 // DEBUG: SYNC INDEX FLAG TEST
		char buffer[64];
		drawList->AddCircleFilled(position, 8.0f, IM_COL32_BLACK);
		drawList->AddText(position - vec2(3.0f, 9.0f), IM_COL32_WHITE, buffer, buffer + sprintf_s(buffer, "%d", target.Flags.IndexWithinSyncPair));
#endif
	}

	const Graphics::Spr* TimelineRenderHelper::GetButtonSpriteForTarget(const TimelineTarget& target) const
	{
		const auto typeIndex = static_cast<u8>(target.Type);
		const bool isSync = target.Flags.IsSync;
		bool isFrag = (target.Flags.IsChain && !target.Flags.IsChainStart);

		// NOTE: This does not match the correct behavior as used in the render window but should avoid confusion between single fragment chains and normal slides
		if (target.Flags.IsChainStart && target.Flags.IsChainEnd)
			isFrag = true;

		const auto& typesArray =
			isFrag ? isSync ? sprites.ButtonIconsFragSync : sprites.ButtonIconsFrag :
			isSync ? sprites.ButtonIconsSync : sprites.ButtonIcons;

		return IndexOr(typeIndex, typesArray, nullptr);
	}

	const Graphics::Spr* TimelineRenderHelper::GetHoldTextSprite(const bool isSync) const
	{
		return isSync ? sprites.HoldTextSync : sprites.HoldText;
	}
}
