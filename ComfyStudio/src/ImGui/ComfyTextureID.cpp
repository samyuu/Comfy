#include "ComfyTextureID.h"
#include "Graphics/D3D11/Texture/RenderTarget.h"
#include "Graphics/D3D11/Texture/Texture.h"

namespace Comfy
{
	using namespace Graphics;

	ComfyTextureID::ComfyTextureID(const nullptr_t dummy)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(dummy);
	}

	ComfyTextureID::ComfyTextureID(const D3D_TextureResource& texture)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(texture.GetResourceView());
		Data.DecompressRGTC = texture.GetTextureFormat() == TextureFormat::RGTC2;
		Data.IsCubeMap = texture.GetArraySize() == 6;
	}

	ComfyTextureID::ComfyTextureID(const D3D_RenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(renderTarget.GetResourceView());
	}

	ComfyTextureID::ComfyTextureID(const D3D_DepthOnlyRenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(renderTarget.GetResourceView());
	}
}
