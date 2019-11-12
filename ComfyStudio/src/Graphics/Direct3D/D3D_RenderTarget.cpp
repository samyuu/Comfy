#include "D3D_RenderTarget.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
	D3D_RenderTargetBase::D3D_RenderTargetBase()
	{
	}

	void D3D_RenderTargetBase::Bind(D3D_DepthBuffer* depthBuffer)
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { renderTargetView.Get() };

		D3D.Context->OMSetRenderTargets(
			static_cast<UINT>(renderTargetViews.size()), 
			renderTargetViews.data(), 
			(depthBuffer == nullptr) ? nullptr : depthBuffer->GetDepthStencilView());
	}

	void D3D_RenderTargetBase::UnBind()
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };

		D3D.Context->OMSetRenderTargets(
			static_cast<UINT>(renderTargetViews.size()),
			renderTargetViews.data(),
			nullptr);
	}

	void D3D_RenderTargetBase::Clear(const vec4& color)
	{
		D3D.Context->ClearRenderTargetView(renderTargetView.Get(), glm::value_ptr(color));
	}

	D3D_RenderTarget::D3D_RenderTarget(ivec2 size)
	{
		// TODO:
		// renderTargetViewDescription.Buffer
		// backBufferDescription.Width

		assert(false);
	}

	void D3D_RenderTarget::Resize(ivec2 newSize)
	{
		// TODO:
		assert(false);
	}

	ivec2 D3D_RenderTarget::GetSize() const
	{
		return ivec2(backBufferDescription.Width, backBufferDescription.Height);
	}

	D3D_SwapChainRenderTarget::D3D_SwapChainRenderTarget(IDXGISwapChain* swapChain)
		: swapChain(swapChain)
	{
		swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
	}

	void D3D_SwapChainRenderTarget::Resize(ivec2 newSize)
	{
		backBuffer = nullptr;
		renderTargetView = nullptr;

		D3D.SwapChain->ResizeBuffers(0, newSize.x, newSize.y, DXGI_FORMAT_UNKNOWN, 0);

		swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
	}
}
