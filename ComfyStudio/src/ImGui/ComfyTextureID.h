#pragma once
#include "Types.h"

struct ID3D11ShaderResourceView;

namespace Graphics
{
	class D3D_TextureResource;
	class D3D_RenderTarget;
}

// NOTE: Value struct wrapper around a resource view with some additional data to avoid stale references
struct ComfyTextureID
{
	ID3D11ShaderResourceView* ResourceView;
	bool IsCubeMap;
	bool DecompressRGTC;
	bool Reserved[2];

	ComfyTextureID(const nullptr_t dummy = nullptr);
	ComfyTextureID(const Graphics::D3D_TextureResource& texture);
	ComfyTextureID(const Graphics::D3D_RenderTarget& renderTarget);

	bool operator==(const ComfyTextureID& other) const;
	bool operator!=(const ComfyTextureID& other) const;

	// NOTE: To make sure this struct can still be hashes the same way as the original pointer placeholder
	inline operator intptr_t() const { return reinterpret_cast<intptr_t>(ResourceView); };

	inline operator ID3D11ShaderResourceView*() const { return ResourceView; };
};
