#include "TexExtensions.h"
#include "ImguiExtensions.h"

using namespace Comfy::Graphics;

namespace ImGui
{
	void ImageSprTex(const Tex* tex, std::optional<vec2> size)
	{
		if (tex == nullptr || (tex->GPU_Texture2D == nullptr && tex->GPU_CubeMap == nullptr))
			return;

		const auto adjustedSize = size.value_or(tex->GetSize());
		Image(*tex, adjustedSize, UV0, UV1);
	}

	void ImageObjTex(const Tex* tex, std::optional<vec2> size)
	{
		if (tex == nullptr || (tex->GPU_Texture2D == nullptr && tex->GPU_CubeMap == nullptr))
			return;

		const auto adjustedSize = size.value_or(tex->GetSize());

		ImTextureID textureID = *tex;
		textureID.Data.DecompressRGTC = false;

		if (tex->GetSignature() == TxpSig::CubeMap)
			Image(textureID, adjustedSize, UV0, UV1);
		else
			Image(textureID, adjustedSize, UV0_R, UV1_R);
	}
}
