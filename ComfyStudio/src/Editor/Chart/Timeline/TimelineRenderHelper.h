#pragma once
#include "Types.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Editor/Chart/SortedTargetList.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class TimelineRenderHelper : NonCopyable
	{
	public:
		void OnEditorSpritesLoaded(const Graphics::SprSet* sprSet);
		void DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, f32 scale, f32 transparency = 1.0f) const;

	private:
		const Graphics::Spr* GetButtonSpriteForTarget(const TimelineTarget& target) const;
		const Graphics::Spr* GetHoldTextSprite(const bool isSync) const;

	private:
		static constexpr f32 buttonIconWidth = 52;
		static constexpr f32 sameTypeSyncStackScale = 0.8f;
		static constexpr vec2 sameTypeSyncStackOffset = vec2(4.0f);

		const Graphics::SprSet* editorSprites = nullptr;

		struct SpriteCache
		{
			std::array<const Graphics::Spr*, EnumCount<ButtonType>()>
				ButtonIcons,
				ButtonIconsSync,
				ButtonIconsFrag,
				ButtonIconsFragSync;

			const Graphics::Spr
				*HoldText,
				*HoldTextSync;
		} sprites = {};
	};
}
