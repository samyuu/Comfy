#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "D3D_Texture.h"
#include "D3D_DepthBuffer.h"

namespace Graphics
{
	// NOTE: Since the render target is stretched to the correct asspect ratio in the end it could easily be scaled down to improve performance
	constexpr ivec2 RenderTargetDefaultSize = D3D_Texture2D::MinSize;

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
		~D3D_SwapChainRenderTarget() = default;

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
		~D3D_RenderTarget() = default;

	public:
		void BindResource(uint32_t textureSlot);

		ivec2 GetSize() const override;
		void Resize(ivec2 newSize) override;
		void SetFormat(DXGI_FORMAT format);

		uint32_t GetMultiSampleCount() const;

		ID3D11Resource* GetResource() const;
		ID3D11ShaderResourceView* GetResourceView() const override;

		UniquePtr<uint8_t[]> StageAndCopyBackBuffer();

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
		~D3D_DepthRenderTarget() = default;

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

	class D3D_DepthOnlyRenderTarget final : public D3D_RenderTargetBase, public D3D_ShaderResourceView
	{
	public:
		D3D_DepthOnlyRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat);
		~D3D_DepthOnlyRenderTarget() = default;

	public:
		void Bind() const override;
		void UnBind() const override;

		void Clear(const vec4& color) override;

		ivec2 GetSize() const override;
		void Resize(ivec2 newSize) override;

		void BindResource(uint32_t textureSlot);

		ID3D11ShaderResourceView* GetResourceView() const override;
		D3D_ResourceViewDepthBuffer* GetDepthBuffer();

	private:
		D3D_ResourceViewDepthBuffer resourceViewDepthBuffer;
	};
}
