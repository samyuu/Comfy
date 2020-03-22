#pragma once
#include "TxpExtensions.h"
#include "ImguiExtensions.h"

using namespace Comfy::Graphics;

namespace ImGui
{
	void ImageSprTxp(const Txp* txp, const ImVec2& size)
	{
		D3D_TextureResource* textureResource = txp->GPU_Texture2D.get();

		if (textureResource == nullptr)
			return;

		const vec2 resourceSize = textureResource->GetSize();
		const vec2 adjustedSize = vec2((size.x <= 0.0f) ? resourceSize.x : size.x, (size.y <= 0.0f) ? resourceSize.y : size.y);

		Image(*textureResource, adjustedSize, UV0, UV1);
	}

	void ImageObjTxp(const Txp* txp, const ImVec2& size)
	{
		D3D_TextureResource* textureResource = txp->GPU_Texture2D.get();

		if (textureResource == nullptr)
			textureResource = txp->GPU_CubeMap.get();

		if (textureResource == nullptr)
			return;

		const vec2 resourceSize = textureResource->GetSize();
		const vec2 adjustedSize = vec2((size.x <= 0.0f) ? resourceSize.x : size.x, (size.y <= 0.0f) ? resourceSize.y : size.y);

		ImTextureID textureID = *textureResource;
		textureID.DecompressRGTC = false;

		if (textureResource == txp->GPU_CubeMap.get())
			Image(textureID, adjustedSize, UV0, UV1);
		else
			Image(textureID, adjustedSize, UV0_R, UV1_R);
	}
}
