#include "RenderWindow.h"
#include "Render/D3D11/Texture/RenderTarget.h"

namespace Comfy
{
	struct RenderWindow2D::Impl
	{
		std::unique_ptr<Render::D3D11::RenderTarget> RenderTarget = nullptr;
	};

	RenderWindow2D::RenderWindow2D() : impl(std::make_unique<Impl>())
	{
	}

	RenderWindow2D::~RenderWindow2D()
	{
	}

	ImTextureID RenderWindow2D::GetTextureID() const
	{
		return (impl->RenderTarget == nullptr) ? nullptr : ImTextureID(*impl->RenderTarget);
	}

	ImGuiWindowFlags RenderWindow2D::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void RenderWindow2D::OnFirstFrame()
	{
		constexpr auto multiSampleCount = 4;
		impl->RenderTarget = std::make_unique<Render::D3D11::RenderTarget>(Render::D3D11::RenderTargetDefaultSize, Render::D3D11::RenderTargetLDRFormatRGBA, multiSampleCount);
	}

	void RenderWindow2D::PreRenderTextureGui()
	{
		// Gui::TextUnformatted(__FUNCTION__);
	}

	void RenderWindow2D::PostRenderTextureGui()
	{
		// Gui::TextUnformatted(__FUNCTION__);
	}

	void RenderWindow2D::OnResize(ivec2 newSize)
	{
		impl->RenderTarget->ResizeIfDifferent(newSize);
	}

	void RenderWindow2D::OnRender()
	{
		impl->RenderTarget->BindSetViewport();
		{
			impl->RenderTarget->Clear(vec4(0.84, 0.67, 0.41, 1.0f));

			// TODO:
			OnRenderDebugFunc();
		}
		impl->RenderTarget->UnBind();
	}
}
