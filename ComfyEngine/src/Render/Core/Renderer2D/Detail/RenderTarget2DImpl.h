#pragma once
#include "Render/Core/Renderer2D/RenderTarget2D.h"
#include "Render/D3D11/Texture/RenderTarget.h"

namespace Comfy::Render::Detail
{
	class RenderTarget2DImpl final : public RenderTarget2D
	{
	public:
		RenderTarget2DImpl() = default;
		~RenderTarget2DImpl() = default;

	public:
		ComfyTextureID GetTextureID() const override
		{
			return (Main.GetMultiSampleCount() > 1) ? ResolvedMain : Main;
		}

	public:
		std::unique_ptr<u8[]> TakeScreenshot() override
		{
			return (Main.GetMultiSampleCount() > 1) ? ResolvedMain.StageAndCopyBackBuffer() : Main.StageAndCopyBackBuffer();
		}

	public:
		D3D11::RenderTarget Main = D3D11::RenderTarget(Param.Resolution);
		D3D11::RenderTarget ResolvedMain = D3D11::RenderTarget(Param.Resolution);
	};
}
