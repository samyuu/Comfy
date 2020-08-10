#include "TimelineButtonIcons.h"
#include "System/ComfyData.h"

namespace Comfy::Studio::Editor
{
	TimelineButtonIcons::TimelineButtonIcons()
	{
		LoadInitializeTexture();
	}

	void TimelineButtonIcons::DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, f32 scale, f32 transparency) const
	{
		const auto width = buttonIconWidth * scale;
		const auto height = buttonIconWidth * scale;

#if 0
		position.x = glm::round(position.x - (width * 0.5f));
		position.y = glm::round(position.y - (height * 0.5f));
#else
		position.x = (position.x - (width * 0.5f));
		position.y = (position.y - (height * 0.5f));
#endif

		const auto bottomRight = vec2(position.x + width, position.y + height);
		const auto textureCoordinates = buttonIconsTextureCoordinates[GetTargetButtonIconIndex(target)];

		const auto color = IM_COL32(0xFF, 0xFF, 0xFF, 0xFF * transparency);
		if (buttonIconsTexture != nullptr)
			drawList->AddImage(*buttonIconsTexture, position, bottomRight, textureCoordinates.GetBL(), textureCoordinates.GetTR(), color);

		if (target.Flags.IsHold)
		{
			// TODO: draw tgt_txt
		}
	}

	size_t TimelineButtonIcons::GetTargetButtonIconIndex(const TimelineTarget& target) const
	{
		auto resultIndex = static_cast<size_t>(target.Type);

		if (target.Flags.IsChain && (target.Type == ButtonType::SlideL || target.Type == ButtonType::SlideR))
			resultIndex += 2;

		if (target.Flags.IsSync)
			return resultIndex;

		return resultIndex + 8;
	}

	void TimelineButtonIcons::LoadInitializeTexture()
	{
		// TODO: Clean all of this up and do sprite name lookups instead of enum integer mapping
		// sankaku		| shikaku		| batsu		 | maru		 | slide_l		| slide_r	   | slide_chain_l		| slide_chain_r
		// sankaku_sync | shikaku_sync  | batsu_sync | maru_sync | slide_l_sync | slide_r_sync | slide_chain_l_sync | slide_chain_r_sync

		// TODO: Load async
		sprSet = System::Data.Load<Graphics::SprSet>(System::Data.FindFile("spr/spr_comfy_editor.bin"));
		if (sprSet == nullptr || sprSet->TexSet.Textures.empty())
			return;

		buttonIconsTexture = sprSet->TexSet.Textures.front();

		const auto texelSize = vec2(1.0f, 1.0f) / vec2(buttonIconsTexture->GetSize());

		const auto width = buttonIconWidth * texelSize.x;
		const auto height = buttonIconWidth * texelSize.y;

		for (size_t i = 0; i < buttonIconsTextureCoordinates.size(); i++)
		{
			const auto x = (buttonIconWidth * (i % buttonIconsTypeCount)) * texelSize.x;
			const auto y = (buttonIconWidth * (i / buttonIconsTypeCount)) * texelSize.y;

			buttonIconsTextureCoordinates[i] = ImRect(x, y, x + width, y + height);
		}
	}
}
