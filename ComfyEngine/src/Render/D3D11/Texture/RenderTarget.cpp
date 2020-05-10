#include "RenderTarget.h"

namespace Comfy::Graphics::D3D11
{
	namespace
	{
		std::unique_ptr<u8[]> StageAndCopyD3DTexture2D(ID3D11Texture2D* sourceTexture, D3D11_TEXTURE2D_DESC textureDescription)
		{
			assert(sourceTexture != nullptr && textureDescription.Format == DXGI_FORMAT_R8G8B8A8_UNORM);

			textureDescription.Usage = D3D11_USAGE_STAGING;
			textureDescription.BindFlags = 0;
			textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			textureDescription.MiscFlags = 0;

			ComPtr<ID3D11Texture2D> stagingTexture;
			if (FAILED(D3D.Device->CreateTexture2D(&textureDescription, nullptr, &stagingTexture)) || stagingTexture == nullptr)
				return nullptr;

			D3D.Context->CopyResource(stagingTexture.Get(), sourceTexture);

			constexpr size_t bytesPerPixel = 4;
			constexpr size_t bitsPerPixel = bytesPerPixel * CHAR_BIT;

			// NOTE: Not sure if the stride calculations are entirely correct but it seems to work fine from my tests
			const size_t dataSize = (textureDescription.Width * textureDescription.Height * bytesPerPixel);
			const size_t strideSize = textureDescription.Width + (bitsPerPixel - (textureDescription.Width % bitsPerPixel));
			const size_t paddedDataSize = (strideSize * textureDescription.Height * bytesPerPixel);

			auto data = std::make_unique<u8[]>(paddedDataSize);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (FAILED(D3D.Context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource)))
				return nullptr;

			std::memcpy(data.get(), mappedResource.pData, dataSize);
			D3D.Context->Unmap(stagingTexture.Get(), 0);

			return data;
		}
	}

	RenderTargetBase::RenderTargetBase()
	{
	}

	void RenderTargetBase::Bind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { renderTargetView.Get() };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void RenderTargetBase::BindSetViewport() const
	{
		Bind();
		D3D.SetViewport(GetSize());
	}

	void RenderTargetBase::UnBind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void RenderTargetBase::Clear(const vec4& color)
	{
		D3D.Context->ClearRenderTargetView(renderTargetView.Get(), glm::value_ptr(color));
	}

	void RenderTargetBase::ResizeIfDifferent(ivec2 newSize)
	{
		newSize = glm::clamp(newSize, Texture2D::MinSize, Texture2D::MaxSize);

		if (newSize != GetSize())
			Resize(newSize);
	}

	SwapChainRenderTarget::SwapChainRenderTarget(IDXGISwapChain* swapChain)
		: swapChain(swapChain)
	{
		swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

		D3D11_TEXTURE2D_DESC backBufferDescription;
		backBuffer->GetDesc(&backBufferDescription);

		size.x = backBufferDescription.Width;
		size.y = backBufferDescription.Height;
	}

	void SwapChainRenderTarget::Resize(ivec2 newSize)
	{
		backBuffer = nullptr;
		renderTargetView = nullptr;

		D3D.SwapChain->ResizeBuffers(0, newSize.x, newSize.y, DXGI_FORMAT_UNKNOWN, 0);
		size = newSize;

		swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
	}

	ivec2 SwapChainRenderTarget::GetSize() const
	{
		return size;
	}

	RenderTarget::RenderTarget(ivec2 size)
		: RenderTarget(size, RenderTargetLDRFormatRGBA)
	{
	}

	RenderTarget::RenderTarget(ivec2 size, DXGI_FORMAT format, u32 multiSampleCount)
	{
		backBufferDescription.Width = size.x;
		backBufferDescription.Height = size.y;
		backBufferDescription.MipLevels = 1;
		backBufferDescription.ArraySize = 1;
		backBufferDescription.Format = format;
		backBufferDescription.SampleDesc.Count = multiSampleCount;
		backBufferDescription.SampleDesc.Quality = 0;
		backBufferDescription.Usage = D3D11_USAGE_DEFAULT;
		backBufferDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		backBufferDescription.CPUAccessFlags = 0;
		backBufferDescription.MiscFlags = 0;

		D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);

		renderTargetViewDescription.Format = backBufferDescription.Format;
		renderTargetViewDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
		renderTargetViewDescription.Texture2D.MipSlice = 0;

		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);

		shaderResourceViewDescription.Format = backBufferDescription.Format;
		shaderResourceViewDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
		shaderResourceViewDescription.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDescription.Texture2D.MipLevels = 1;

		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	void RenderTarget::BindResource(u32 textureSlot)
	{
		std::array<ID3D11ShaderResourceView*, 1> resourceViews = { GetResourceView() };
		D3D.Context->PSSetShaderResources(textureSlot, static_cast<UINT>(resourceViews.size()), resourceViews.data());
	}

	ivec2 RenderTarget::GetSize() const
	{
		return ivec2(backBufferDescription.Width, backBufferDescription.Height);
	}

	void RenderTarget::Resize(ivec2 newSize)
	{
		backBufferDescription.Width = newSize.x;
		backBufferDescription.Height = newSize.y;

		D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);
		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	void RenderTarget::SetFormat(DXGI_FORMAT format)
	{
		if (backBufferDescription.Format == format)
			return;

		backBufferDescription.Format = format;
		renderTargetViewDescription.Format = backBufferDescription.Format;
		shaderResourceViewDescription.Format = backBufferDescription.Format;
		D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);
		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	u32 RenderTarget::GetMultiSampleCount() const
	{
		return backBufferDescription.SampleDesc.Count;
	}

	ID3D11Resource* RenderTarget::GetResource() const
	{
		return backBuffer.Get();
	}

	ID3D11ShaderResourceView* RenderTarget::GetResourceView() const
	{
		return shaderResourceView.Get();
	}

	const D3D11_TEXTURE2D_DESC& RenderTarget::GetBackBufferDescription() const
	{
		return backBufferDescription;
	}

	std::unique_ptr<u8[]> RenderTarget::StageAndCopyBackBuffer()
	{
		return StageAndCopyD3DTexture2D(backBuffer.Get(), backBufferDescription);
	}

	DepthRenderTarget::DepthRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat)
		: RenderTarget(size), depthBuffer(size, depthBufferFormat)
	{
	}

	DepthRenderTarget::DepthRenderTarget(ivec2 size, DXGI_FORMAT format, DXGI_FORMAT depthBufferFormat, u32 multiSampleCount)
		: RenderTarget(size, format, multiSampleCount), depthBuffer(size, depthBufferFormat, multiSampleCount)
	{
	}

	void DepthRenderTarget::Bind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { renderTargetView.Get() };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), depthBuffer.GetDepthStencilView());
	}

	void DepthRenderTarget::UnBind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void DepthRenderTarget::Clear(const vec4& color)
	{
		RenderTarget::Clear(color);
		depthBuffer.Clear();
	}

	void DepthRenderTarget::Resize(ivec2 newSize)
	{
		RenderTarget::Resize(newSize);
		depthBuffer.Resize(newSize);
	}

	void DepthRenderTarget::SetMultiSampleCount(u32 multiSampleCount)
	{
		backBufferDescription.SampleDesc.Count = multiSampleCount;
		backBufferDescription.SampleDesc.Quality = 0;
		if FAILED(D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer))
		{
			backBufferDescription.SampleDesc.Count = multiSampleCount = 1;
			D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);
		}

		renderTargetViewDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);

		shaderResourceViewDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);

		depthBuffer.SetMultiSampleCount(multiSampleCount);
	}

	void DepthRenderTarget::SetMultiSampleCountIfDifferent(u32 multiSampleCount)
	{
		if (multiSampleCount != GetMultiSampleCount())
			SetMultiSampleCount(multiSampleCount);
	}

	DepthBuffer* DepthRenderTarget::GetDepthBuffer()
	{
		return &depthBuffer;
	}

	DepthOnlyRenderTarget::DepthOnlyRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat)
		: resourceViewDepthBuffer(size, depthBufferFormat)
	{
	}

	void DepthOnlyRenderTarget::Bind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), resourceViewDepthBuffer.GetDepthStencilView());
	}

	void DepthOnlyRenderTarget::UnBind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void DepthOnlyRenderTarget::Clear(const vec4& color)
	{
		resourceViewDepthBuffer.Clear();
	}

	ivec2 DepthOnlyRenderTarget::GetSize() const
	{
		return resourceViewDepthBuffer.GetSize();
	}

	void DepthOnlyRenderTarget::Resize(ivec2 newSize)
	{
		resourceViewDepthBuffer.Resize(newSize);
	}

	void DepthOnlyRenderTarget::BindResource(u32 textureSlot)
	{
		std::array<ID3D11ShaderResourceView*, 1> resourceViews = { GetResourceView() };
		D3D.Context->PSSetShaderResources(textureSlot, static_cast<UINT>(resourceViews.size()), resourceViews.data());
	}

	ID3D11ShaderResourceView* DepthOnlyRenderTarget::GetResourceView() const
	{
		return resourceViewDepthBuffer.GetResourceView();
	}

	ResourceViewDepthBuffer* DepthOnlyRenderTarget::GetDepthBuffer()
	{
		return &resourceViewDepthBuffer;
	}
}
