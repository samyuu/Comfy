#include "ComfyTextureID.h"
#include "Render/D3D11/D3D11Texture.h"
#include "Render/D3D11/D3D11OpaqueResource.h"

namespace Comfy
{
	using namespace Graphics;

	ComfyTextureID::ComfyTextureID(ID3D11ShaderResourceView* resourceView)
	{
		Data.ResourceView = reinterpret_cast<u64>(resourceView);
	}

	ComfyTextureID::ComfyTextureID(const Tex& tex)
	{
		Data.ResourceView = reinterpret_cast<u64>(Render::GetD3D11Texture2DView(Render::GlobalD3D11, tex));
		Data.DecompressRGTC = (tex.GetFormat() == TextureFormat::RGTC2);
		Data.IsCubeMap = (tex.GetSignature() == TxpSig::CubeMap);
	}

	ComfyTextureID::ComfyTextureID(const LightMapIBL& lightMap)
	{
		Data.ResourceView = reinterpret_cast<u64>(Render::GetD3D11Texture2DView(Render::GlobalD3D11, lightMap));
		Data.IsCubeMap = true;
	}

	ComfyTextureID::ComfyTextureID(const Render::D3D11Texture2DAndView& texture)
	{
		Data.ResourceView = reinterpret_cast<u64>(texture.TextureView.Get());
		Data.DecompressRGTC = (texture.TextureFormat == TextureFormat::RGTC2);
		Data.IsCubeMap = texture.GetIsCubeMap();
	}

	ComfyTextureID::ComfyTextureID(const Render::D3D11RenderTargetAndView& renderTarget)
	{
		Data.ResourceView = reinterpret_cast<u64>(renderTarget.ColorTextureView.Get());
	}

	ComfyTextureID::ComfyTextureID(const Render::D3D11RenderTargetAndView& renderTarget, bool depthBuffer)
	{
		Data.ResourceView = reinterpret_cast<u64>(depthBuffer ? renderTarget.DepthTextureView.Get() : renderTarget.ColorTextureView.Get());
	}
}
