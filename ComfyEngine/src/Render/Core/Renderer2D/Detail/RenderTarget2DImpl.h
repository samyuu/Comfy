#pragma once
#include "Render/Core/Renderer2D/RenderTarget2D.h"

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
			return (Main != nullptr) ? (*Main) : ComfyTextureID(nullptr);
		}

	public:
		std::unique_ptr<D3D11::RenderTarget> Main = std::make_unique<D3D11::RenderTarget>(Param.Resolution);
	};
}
