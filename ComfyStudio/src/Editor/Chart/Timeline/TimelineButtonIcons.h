#pragma once
#include "Types.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Editor/Chart/SortedTargetList.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class TimelineButtonIcons : NonCopyable
	{
	public:
		TimelineButtonIcons();
		~TimelineButtonIcons() = default;

	public:
		void DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, f32 scale, f32 transparency = 1.0f) const;
		size_t GetTargetButtonIconIndex(const TimelineTarget& target) const;

	private:
		void LoadInitializeTexture();

	private:
		// NOTE: sankaku | shikaku | batsu | maru | slide_l | slide_r | slide_chain_l | slide_chain_r
		static constexpr int buttonIconsTypeCount = 8;
		static constexpr int buttonIconWidth = 52;

		std::array<ImRect, buttonIconsTypeCount * 2> buttonIconsTextureCoordinates;

		std::unique_ptr<Graphics::SprSet> sprSet;
		std::shared_ptr<Graphics::Tex> buttonIconsTexture = nullptr;
	};
}
