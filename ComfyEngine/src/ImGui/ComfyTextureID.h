#pragma once
#include "Types.h"

#define COMFY_PACKED_TEXTURE_ID 1

struct ID3D11ShaderResourceView;

namespace Comfy::Graphics
{
	class Tex;
	struct LightMapIBL;
}

namespace Comfy::Render
{
	struct D3D11Texture2DAndView;
	struct D3D11RenderTargetAndView;
}

namespace Comfy
{
	namespace ComfyTextureIDLayouts
	{
		struct Default
		{
			u64 ResourceView;
			u8 DecompressRGTC;
			u8 IsCubeMap;
			u8 CubeMapMipLevel;
			u8 Padding[5];
		};

		struct Packed
		{
			// NOTE: This should be valid according to https://en.wikipedia.org/wiki/Intel_5-level_paging
			u64 ResourceView : 58;
			u64 DecompressRGTC : 1;
			u64 IsCubeMap : 1;
			u64 CubeMapMipLevel : 4;
		};
	}

	// TODO: Replace with something like TextureViewAndSampler2D / Comfy::Render::TexSamplerView (?)

	// NOTE: Value struct wrapper around a resource view with some additional data to avoid stale references
	struct ComfyTextureID
	{
#if COMFY_PACKED_TEXTURE_ID
		using DataLayout = ComfyTextureIDLayouts::Packed;
#else
		using DataLayout = ComfyTextureIDLayouts::Default;
#endif /* COMFY_PACKED_TEXTURE_ID */

		DataLayout Data = {};

		ComfyTextureID(ID3D11ShaderResourceView* resourceView = nullptr);
		ComfyTextureID(const Graphics::Tex& tex);
		ComfyTextureID(const Graphics::LightMapIBL& lightMap);
		ComfyTextureID(const Render::D3D11Texture2DAndView& texture);
		ComfyTextureID(const Render::D3D11RenderTargetAndView& renderTarget);
		ComfyTextureID(const Render::D3D11RenderTargetAndView& renderTarget, bool depthBuffer);

		inline ID3D11ShaderResourceView* GetResourceView() const { return reinterpret_cast<ID3D11ShaderResourceView*>(Data.ResourceView); }

		inline bool operator==(const ComfyTextureID& other) const { return std::memcmp(&Data, &other.Data, sizeof(Data)) == 0; }
		inline bool operator!=(const ComfyTextureID& other) const { return !(*this == other); }

		// NOTE: To make sure this struct can still be hashes the same way as the original pointer placeholder
		inline operator intptr_t() const { return reinterpret_cast<intptr_t>(GetResourceView()); }
	};
}
