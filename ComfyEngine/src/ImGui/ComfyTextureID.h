#pragma once
#include "Types.h"

struct ID3D11ShaderResourceView;

namespace Comfy::Render::D3D11
{
	class TextureResource;
	class RenderTarget;
	class DepthOnlyRenderTarget;
}

namespace Comfy::Graphics
{
	class Tex;
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

#define COMFY_PACKED_TEXTURE_ID 1

	// NOTE: Value struct wrapper around a resource view with some additional data to avoid stale references
	struct ComfyTextureID
	{
#if COMFY_PACKED_TEXTURE_ID
		using DataLayout = ComfyTextureIDLayouts::Packed;
#else
		using DataLayout = ComfyTextureIDLayouts::Default;
#endif /* COMFY_PACKED_TEXTURE_ID */

		DataLayout Data = {};

		ComfyTextureID(const nullptr_t dummy = nullptr);
		ComfyTextureID(const Graphics::Tex& tex);
		ComfyTextureID(const Render::D3D11::TextureResource& texture);
		ComfyTextureID(const Render::D3D11::RenderTarget& renderTarget);
		ComfyTextureID(const Render::D3D11::DepthOnlyRenderTarget& renderTarget);

		inline ID3D11ShaderResourceView* GetResourceView() const { return reinterpret_cast<ID3D11ShaderResourceView*>(Data.ResourceView); }

		inline bool operator==(const ComfyTextureID& other) const { return std::memcmp(&Data, &other.Data, sizeof(Data)) == 0; }
		inline bool operator!=(const ComfyTextureID& other) const { return !(*this == other); }

		// NOTE: To make sure this struct can still be hashes the same way as the original pointer placeholder
		inline operator intptr_t() const { return reinterpret_cast<intptr_t>(GetResourceView()); }
	};
}
