#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "D3D_DepthBuffer.h"

namespace Graphics
{
	class D3D_RenderTargetBase : IGraphicsResource
	{
	protected:
		D3D_RenderTargetBase();
		virtual ~D3D_RenderTargetBase() = default;

	public:
		void Bind(D3D_DepthBuffer* depthBuffer = nullptr);
		void UnBind();

		virtual void Clear(const vec4& color);
		virtual void Resize(ivec2 newSize) = 0;

	protected:
		ComPtr<ID3D11Texture2D> backBuffer;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
	};

	class D3D_RenderTarget final : public D3D_RenderTargetBase
	{
	public:
		D3D_RenderTarget(ivec2 size);
		D3D_RenderTarget(const D3D_RenderTarget&) = delete;
		~D3D_RenderTarget() = default;

		D3D_RenderTarget& operator=(const D3D_RenderTarget&) = delete;

	public:
		void Resize(ivec2 newSize) override;

		ivec2 GetSize() const;

	private:
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription;
		D3D11_TEXTURE2D_DESC backBufferDescription;
	};

	class D3D_SwapChainRenderTarget final : public D3D_RenderTargetBase
	{
	public:
		D3D_SwapChainRenderTarget(IDXGISwapChain* swapChain);
		D3D_SwapChainRenderTarget(const D3D_SwapChainRenderTarget&) = delete;
		~D3D_SwapChainRenderTarget() = default;

		D3D_SwapChainRenderTarget& operator=(const D3D_SwapChainRenderTarget&) = delete;

	public:
		void Resize(ivec2 newSize) override;

	private:
		IDXGISwapChain* swapChain;
	};
}
