#include "ComfyTextureID.h"
#include "Graphics/Direct3D/Texture/D3D_RenderTarget.h"
#include "Graphics/Direct3D/Texture/D3D_Texture.h"

namespace Comfy
{
	using namespace Graphics;

	inline ComfyTextureID::ComfyTextureID_T(const nullptr_t dummy)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(dummy);
	}

	inline ComfyTextureID::ComfyTextureID_T(const D3D_TextureResource& texture)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(texture.GetResourceView());
		Data.DecompressRGTC = texture.GetTextureFormat() == TextureFormat::RGTC2;
		Data.IsCubeMap = texture.GetArraySize() == 6;
	}

	inline ComfyTextureID::ComfyTextureID_T(const D3D_RenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(renderTarget.GetResourceView());
	}

	inline ComfyTextureID::ComfyTextureID_T(const D3D_DepthOnlyRenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<uint64_t>(renderTarget.GetResourceView());
	}
}
