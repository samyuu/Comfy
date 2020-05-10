#pragma once
#include "Types.h"
#include "../Direct3D.h"
#include "DepthBuffer.h"
#include "Texture.h"

namespace Comfy::Graphics::D3D11
{
	// NOTE: Since the render target is stretched to the correct asspect ratio in the end it could easily be scaled down to improve performance
	constexpr ivec2 RenderTargetDefaultSize = Texture2D::MinSize;

	constexpr DXGI_FORMAT RenderTargetHDRFormatRGBA = DXGI_FORMAT_R16G16B16A16_FLOAT;
	constexpr DXGI_FORMAT RenderTargetLDRFormatRGBA = DXGI_FORMAT_R8G8B8A8_UNORM;

	class RenderTargetBase : IGraphicsResource
	{
	protected:
		RenderTargetBase();
		virtual ~RenderTargetBase() = default;

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

	class SwapChainRenderTarget final : public RenderTargetBase
	{
	public:
		SwapChainRenderTarget(IDXGISwapChain* swapChain);
		~SwapChainRenderTarget() = default;

	public:
		void Resize(ivec2 newSize) override;
		ivec2 GetSize() const override;

	private:
		ivec2 size;
		IDXGISwapChain* swapChain;
	};

	class RenderTarget : public RenderTargetBase, public ShaderResourceView
	{
	public:
		RenderTarget(ivec2 size);
		RenderTarget(ivec2 size, DXGI_FORMAT format, u32 multiSampleCount = 1);
		~RenderTarget() = default;

	public:
		void BindResource(u32 textureSlot);

		ivec2 GetSize() const override;
		void Resize(ivec2 newSize) override;
		void SetFormat(DXGI_FORMAT format);

		u32 GetMultiSampleCount() const;

		ID3D11Resource* GetResource() const;
		ID3D11ShaderResourceView* GetResourceView() const override;

		const D3D11_TEXTURE2D_DESC& GetBackBufferDescription() const;

		std::unique_ptr<u8[]> StageAndCopyBackBuffer();

	protected:
		D3D11_TEXTURE2D_DESC backBufferDescription;
		D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDescription;
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;

		ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	};

	class DepthRenderTarget final : public RenderTarget
	{
	public:
		DepthRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat);
		DepthRenderTarget(ivec2 size, DXGI_FORMAT format, DXGI_FORMAT depthBufferFormat, u32 multiSampleCount = 1);
		~DepthRenderTarget() = default;

	public:
		void Bind() const override;
		void UnBind() const override;

		void Clear(const vec4& color) override;
		void Resize(ivec2 newSize) override;

		void SetMultiSampleCount(u32 multiSampleCount);
		void SetMultiSampleCountIfDifferent(u32 multiSampleCount);

		DepthBuffer* GetDepthBuffer();

	private:
		DepthBuffer depthBuffer;
	};

	class DepthOnlyRenderTarget final : public RenderTargetBase, public ShaderResourceView
	{
	public:
		DepthOnlyRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat);
		~DepthOnlyRenderTarget() = default;

	public:
		void Bind() const override;
		void UnBind() const override;

		void Clear(const vec4& color) override;

		ivec2 GetSize() const override;
		void Resize(ivec2 newSize) override;

		void BindResource(u32 textureSlot);

		ID3D11ShaderResourceView* GetResourceView() const override;
		ResourceViewDepthBuffer* GetDepthBuffer();

	private:
		ResourceViewDepthBuffer resourceViewDepthBuffer;
	};
}
