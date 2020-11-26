#pragma once
#include "Types.h"
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
		ComfyTextureID GetTextureID() const override { return Output; }

	public:
		std::unique_ptr<u8[]> TakeScreenshot() override { return Output.StageAndCopyBackBuffer(); }

	public:
		D3D11::RenderTarget Main = D3D11::RenderTarget(Param.Resolution);
		D3D11::RenderTarget Output = D3D11::RenderTarget(Param.Resolution);
	};
}
