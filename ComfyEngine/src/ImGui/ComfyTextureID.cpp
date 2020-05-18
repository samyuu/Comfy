#include "ComfyTextureID.h"
#include "Graphics/TexSet.h"
#include "Render/D3D11/Texture/RenderTarget.h"
#include "Render/D3D11/Texture/Texture.h"
#include "Render/D3D11/GraphicsResourceUtil.h"

namespace Comfy
{
	using namespace Graphics;

	ComfyTextureID::ComfyTextureID(const nullptr_t dummy)
	{
		Data.ResourceView = reinterpret_cast<u64>(dummy);
	}

	ComfyTextureID::ComfyTextureID(const Tex& tex)
	{
		auto getResourceView = [&](auto* texture) { return (texture != nullptr) ? texture->GetResourceView() : nullptr; };
		auto resourceView = (tex.GetSignature() == TxpSig::Texture2D) ?
			getResourceView(Render::D3D11::GetTexture2D(tex)) :
			getResourceView(Render::D3D11::GetCubeMap(tex));

		Data.ResourceView = reinterpret_cast<u64>(resourceView);
		Data.DecompressRGTC = tex.GetFormat() == TextureFormat::RGTC2;
		Data.IsCubeMap = tex.GetSignature() == TxpSig::CubeMap;
	}

	ComfyTextureID::ComfyTextureID(const Render::D3D11::TextureResource& texture)
	{
		Data.ResourceView = reinterpret_cast<u64>(texture.GetResourceView());
		Data.DecompressRGTC = texture.GetTextureFormat() == TextureFormat::RGTC2;
		Data.IsCubeMap = texture.GetArraySize() == 6;
	}

	ComfyTextureID::ComfyTextureID(const Render::D3D11::RenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<u64>(renderTarget.GetResourceView());
	}

	ComfyTextureID::ComfyTextureID(const Render::D3D11::DepthOnlyRenderTarget& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<u64>(renderTarget.GetResourceView());
	}
}
