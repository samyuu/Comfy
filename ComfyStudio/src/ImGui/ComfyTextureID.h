#pragma once
#include "Types.h"

struct ID3D11ShaderResourceView;

namespace Comfy::Graphics
{
	class D3D_TextureResource;
	class D3D_RenderTarget;
	class D3D_DepthOnlyRenderTarget;
}

namespace Comfy
{
	namespace ComfyTextureIDLayouts
	{
		struct Default
		{
			// ID3D11ShaderResourceView* ResourceView;
			uint64_t ResourceView;
			uint8_t DecompressRGTC;
			uint8_t IsCubeMap;
			uint8_t CubeMapMipLevel;
			uint8_t Padding[5];
		};

		struct Packed
		{
			// NOTE: This should be valid according to https://en.wikipedia.org/wiki/Intel_5-level_paging
			uint64_t ResourceView : 58;
			uint64_t DecompressRGTC : 1;
			uint64_t IsCubeMap : 1;
			uint64_t CubeMapMipLevel : 4;
		};
	}

	// NOTE: Value struct wrapper around a resource view with some additional data to avoid stale references
	template <typename DataLayout>
	struct ComfyTextureID_T
	{
		DataLayout Data = {};

		ComfyTextureID_T(const nullptr_t dummy = nullptr);
		ComfyTextureID_T(const Graphics::D3D_TextureResource& texture);
		ComfyTextureID_T(const Graphics::D3D_RenderTarget& renderTarget);
		ComfyTextureID_T(const Graphics::D3D_DepthOnlyRenderTarget& renderTarget);

		inline ID3D11ShaderResourceView* GetResourceView() const { return reinterpret_cast<ID3D11ShaderResourceView*>(Data.ResourceView); }

		bool operator==(const ComfyTextureID_T& other) const { return std::memcmp(&Data, &other.Data, sizeof(Data)) == 0; }
		bool operator!=(const ComfyTextureID_T& other) const { return !(*this == other); }

		// NOTE: To make sure this struct can still be hashes the same way as the original pointer placeholder
		inline operator intptr_t() const { return reinterpret_cast<intptr_t>(GetResourceView()); }
	};

#define COMFY_PACKED_TEXTURE_ID 1

#pragma warning( push )
#pragma warning( disable : 4661 ) // 'identifier' : no suitable definition provided for explicit template instantiation request

#if COMFY_PACKED_TEXTURE_ID
	template struct ComfyTextureID_T<ComfyTextureIDLayouts::Packed>;
	using ComfyTextureID = ComfyTextureID_T<ComfyTextureIDLayouts::Packed>;
#else
	template struct ComfyTextureID_T<ComfyTextureIDLayouts::Default>;
	using ComfyTextureID = ComfyTextureID_T<ComfyTextureIDLayouts::Default>;
#endif /* COMFY_PACKED_TEXTURE_ID */

#pragma warning( pop )

	constexpr size_t SizeOfComfyTextureID = sizeof(ComfyTextureID);
}
