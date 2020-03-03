#include "ComfyTextureID.h"
#include "Graphics/Direct3D/D3D_Texture.h"
#include "Graphics/Direct3D/D3D_RenderTarget.h"

namespace Comfy
{
	using namespace Graphics;

	ComfyTextureID::ComfyTextureID(const nullptr_t dummy)
		: ResourceView(dummy), IsCubeMap(false), DecompressRGTC(false)
	{
	}

	ComfyTextureID::ComfyTextureID(const D3D_TextureResource& texture)
		: ResourceView(texture.GetResourceView()), IsCubeMap(texture.GetArraySize() == 6), DecompressRGTC(!IsCubeMap && texture.GetTextureFormat() == TextureFormat::RGTC2)
	{
	}

	ComfyTextureID::ComfyTextureID(const D3D_RenderTarget& renderTarget)
		: ResourceView(renderTarget.GetResourceView()), IsCubeMap(false), DecompressRGTC(false)
	{
	}

	ComfyTextureID::ComfyTextureID(const D3D_DepthOnlyRenderTarget& renderTarget)
		: ResourceView(renderTarget.GetResourceView()), IsCubeMap(false), DecompressRGTC(false)
	{
	}

	bool ComfyTextureID::operator==(const ComfyTextureID& other) const
	{
		return ResourceView == other.ResourceView;
	}

	bool ComfyTextureID::operator!=(const ComfyTextureID& other) const
	{
		return ResourceView != other.ResourceView;
	}
}
