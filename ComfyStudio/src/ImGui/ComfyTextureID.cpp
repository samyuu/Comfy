#include "ComfyTextureID.h"
#include "Graphics/D3D11/Texture/RenderTarget.h"
#include "Graphics/D3D11/Texture/Texture.h"

namespace Comfy
{
	using namespace Graphics;

	ComfyTextureID::ComfyTextureID(const nullptr_t dummy)
	{
		Data.ResourceView = reinterpret_cast<u64>(dummy);
	}

	ComfyTextureID::ComfyTextureID(const D3D11::TextureResource& texture)
	{
		Data.ResourceView = reinterpret_cast<u64>(texture.GetResourceView());
		Data.DecompressRGTC = texture.GetTextureFormat() == TextureFormat::RGTC2;
		Data.IsCubeMap = texture.GetArraySize() == 6;
	}

	ComfyTextureID::ComfyTextureID(const D3D11::RenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<u64>(renderTarget.GetResourceView());
	}

	ComfyTextureID::ComfyTextureID(const D3D11::DepthOnlyRenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<u64>(renderTarget.GetResourceView());
	}
}
