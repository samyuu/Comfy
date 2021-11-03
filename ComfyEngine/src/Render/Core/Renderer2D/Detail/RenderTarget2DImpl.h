#pragma once
#include "Types.h"
#include "Render/Core/Renderer2D/RenderTarget2D.h"
#include "Render/D3D11/D3D11Texture.h"

namespace Comfy::Render::Detail
{
	class RenderTarget2DImpl final : public RenderTarget2D
	{
	public:
		RenderTarget2DImpl() = default;
		~RenderTarget2DImpl() = default;

	public:
		ComfyTextureID GetTextureID() const override { return Output; }

	public:
		std::unique_ptr<u8[]> TakeScreenshot() override { return Output.CopyColorPixelsBackToCPU(GlobalD3D11); }

	public:
		D3D11RenderTargetAndView Main = D3D11RenderTargetAndView(GlobalD3D11, Param.Resolution, D3D11RenderTargetLDRFormatRGBA);
		D3D11RenderTargetAndView Output = D3D11RenderTargetAndView(GlobalD3D11, Param.Resolution, D3D11RenderTargetLDRFormatRGBA);
	};
}
