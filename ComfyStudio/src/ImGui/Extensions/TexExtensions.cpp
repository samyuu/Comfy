#include "TexExtensions.h"
#include "ImguiExtensions.h"

using namespace Comfy::Graphics;

namespace ImGui
{
	void ImageSprTex(const Tex* tex, const ImVec2& size)
	{
		D3D11::TextureResource* textureResource = tex->GPU_Texture2D.get();

		if (textureResource == nullptr)
			return;

		const vec2 resourceSize = textureResource->GetSize();
		const vec2 adjustedSize = vec2((size.x <= 0.0f) ? resourceSize.x : size.x, (size.y <= 0.0f) ? resourceSize.y : size.y);

		Image(*textureResource, adjustedSize, UV0, UV1);
	}

	void ImageObjTex(const Tex* tex, const ImVec2& size)
	{
		D3D11::TextureResource* textureResource = tex->GPU_Texture2D.get();

		if (textureResource == nullptr)
			textureResource = tex->GPU_CubeMap.get();

		if (textureResource == nullptr)
			return;

		const vec2 resourceSize = textureResource->GetSize();
		const vec2 adjustedSize = vec2((size.x <= 0.0f) ? resourceSize.x : size.x, (size.y <= 0.0f) ? resourceSize.y : size.y);

		ImTextureID textureID = *textureResource;
		textureID.Data.DecompressRGTC = false;

		if (textureResource == tex->GPU_CubeMap.get())
			Image(textureID, adjustedSize, UV0, UV1);
		else
			Image(textureID, adjustedSize, UV0_R, UV1_R);
	}
}
