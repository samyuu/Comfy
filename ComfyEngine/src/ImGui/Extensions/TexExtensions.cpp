#include "TexExtensions.h"
#include "ImguiExtensions.h"

using namespace Comfy::Graphics;

namespace ImGui
{
	namespace
	{
		void FixedAspectRatioImage(ImTextureID textureID, const ImVec2& textureSize, const ImVec2& size, const ImVec2& uv0, const ImVec2& uv1)
		{
			ImGuiWindow* window = GetCurrentWindow();
			if (window->SkipItems)
				return;

			auto bb = ImRect(window->DC.CursorPos, window->DC.CursorPos + size);
			ItemSize(bb);
			if (!ItemAdd(bb, 0))
				return;

			const auto textureBB = FitFixedAspectRatioImage(bb, textureSize);
			window->DrawList->AddImage(textureID, textureBB.Min, textureBB.Max, uv0, uv1);
		}
	}

	void ImageSprTex(const Tex* tex, std::optional<vec2> size)
	{
		auto textureID = (tex != nullptr) ? ImTextureID(*tex) : ImTextureID(nullptr);

		const auto textureSize = (tex != nullptr) ? vec2(tex->GetSize()) : vec2(0.0f);
		const auto adjustedSize = size.value_or(textureSize);

		FixedAspectRatioImage(textureID, textureSize, adjustedSize, UV0, UV1);
	}

	void ImageSprTex(const Tex* tex, const Spr* spr, std::optional<vec2> size)
	{
		auto textureID = (tex != nullptr) ? ImTextureID(*tex) : ImTextureID(nullptr);

		const auto spriteSize = (spr != nullptr) ? vec2(spr->GetSize()) : vec2(0.0f);
		const auto adjustedSize = size.value_or(spriteSize);

		const auto uv = (spr != nullptr) ? std::array { vec2(spr->TexelRegion.x, -spr->TexelRegion.y), vec2(spr->TexelRegion.z, -spr->TexelRegion.w) } : std::array { UV0, UV1 };
		FixedAspectRatioImage(textureID, spriteSize, adjustedSize, uv[0], uv[1]);
	}

	void ImageObjTex(const Tex* tex, std::optional<vec2> size)
	{
		auto textureID = (tex != nullptr) ? ImTextureID(*tex) : ImTextureID(nullptr);
		textureID.Data.DecompressRGTC = false;

		const auto textureSize = (tex != nullptr) ? vec2(tex->GetSize()) : vec2(0.0f);
		const auto adjustedSize = size.value_or(textureSize);

		const auto uv = (textureID.Data.IsCubeMap) ? std::array { UV0, UV1 } : std::array { UV0_R, UV1_R };
		FixedAspectRatioImage(textureID, textureSize, adjustedSize, uv[0], uv[1]);
	}
}
