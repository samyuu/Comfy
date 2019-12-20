#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "D3D_Texture.h"
#include "D3D_DepthBuffer.h"

namespace Graphics
{
	// NOTE: Since the render target is stretched to the correct asspect ratio in the end it could easily be scaled down to improve performance
	constexpr ivec2 RenderTargetDefaultSize = ivec2(1, 1);

	constexpr DXGI_FORMAT RenderTargetHDRFormatRGBA = DXGI_FORMAT_R16G16B16A16_FLOAT;
	constexpr DXGI_FORMAT RenderTargetLDRFormatRGBA = DXGI_FORMAT_R8G8B8A8_UNORM;

	class D3D_RenderTargetBase : IGraphicsResource
	{
	protected:
		D3D_RenderTargetBase();
		virtual ~D3D_RenderTargetBase() = default;

	public:
		virtual void Bind() const;
		void BindSetViewport() const;
		
		virtual void UnBind() const;

		virtual void Clear(const vec4& color);
		virtual ivec2 GetSize() const = 0;
		
		virtual void Resize(ivec2 newSize) = 0;
		virtual void ResizeIfDifferent(ivec2 newSize);

	protected:
		ComPtr<ID3D11Texture2D> backBuffer;
		ComPtr<ID3D11RenderTargetView> renderTargetView;
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
		ivec2 GetSize() const override;

	private:
		ivec2 size;
		IDXGISwapChain* swapChain;
	};

	class D3D_RenderTarget : public D3D_RenderTargetBase, public D3D_ShaderResourceView
	{
	public:
		D3D_RenderTarget(ivec2 size);
		D3D_RenderTarget(ivec2 size, DXGI_FORMAT format, uint32_t multiSampleCount = 1);
		D3D_RenderTarget(const D3D_RenderTarget&) = delete;
		~D3D_RenderTarget() = default;

		D3D_RenderTarget& operator=(const D3D_RenderTarget&) = delete;

	public:
		void BindResource(uint32_t textureSlot);

		ivec2 GetSize() const override;
		void Resize(ivec2 newSize) override;

		uint32_t GetMultiSampleCount() const;
		
		ID3D11ShaderResourceView* GetResourceView() const override;

	protected:
		D3D11_TEXTURE2D_DESC backBufferDescription;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;

		ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	};

	class D3D_DepthRenderTarget final : public D3D_RenderTarget
	{
	public:
		D3D_DepthRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat);
		D3D_DepthRenderTarget(ivec2 size, DXGI_FORMAT format, DXGI_FORMAT depthBufferFormat, uint32_t multiSampleCount = 1);
		D3D_DepthRenderTarget(const D3D_DepthRenderTarget&) = delete;
		~D3D_DepthRenderTarget() = default;

		D3D_DepthRenderTarget& operator=(const D3D_DepthRenderTarget&) = delete;

	public:
		void Bind() const override;
		void UnBind() const override;

		void Clear(const vec4& color) override;
		void Resize(ivec2 newSize) override;

		void SetMultiSampleCount(uint32_t multiSampleCount);
		void SetMultiSampleCountIfDifferent(uint32_t multiSampleCount);

		D3D_DepthBuffer* GetDepthBuffer();

	private:
		D3D_DepthBuffer depthBuffer;
	};
}
