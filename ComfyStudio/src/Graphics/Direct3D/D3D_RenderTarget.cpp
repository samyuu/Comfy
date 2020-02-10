#include "D3D_RenderTarget.h"

namespace Graphics
{
	namespace
	{
		UniquePtr<uint8_t[]> StageAndCopyD3DTexture2D(ID3D11Texture2D* sourceTexture, D3D11_TEXTURE2D_DESC textureDescription)
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

			auto data = MakeUnique<uint8_t[]>(paddedDataSize);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (FAILED(D3D.Context->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource)))
				return nullptr;

			std::memcpy(data.get(), mappedResource.pData, dataSize);
			D3D.Context->Unmap(stagingTexture.Get(), 0);

			return data;
		}
	}

	D3D_RenderTargetBase::D3D_RenderTargetBase()
	{
	}

	void D3D_RenderTargetBase::Bind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { renderTargetView.Get() };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void D3D_RenderTargetBase::BindSetViewport() const
	{
		Bind();
		D3D.SetViewport(GetSize());
	}

	void D3D_RenderTargetBase::UnBind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void D3D_RenderTargetBase::Clear(const vec4& color)
	{
		D3D.Context->ClearRenderTargetView(renderTargetView.Get(), glm::value_ptr(color));
	}

	void D3D_RenderTargetBase::ResizeIfDifferent(ivec2 newSize)
	{
		newSize = glm::clamp(newSize, D3D_Texture2D::MinSize, D3D_Texture2D::MaxSize);

		if (newSize != GetSize())
			Resize(newSize);
	}

	D3D_SwapChainRenderTarget::D3D_SwapChainRenderTarget(IDXGISwapChain* swapChain)
		: swapChain(swapChain)
	{
		swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);

		D3D11_TEXTURE2D_DESC backBufferDescription;
		backBuffer->GetDesc(&backBufferDescription);

		size.x = backBufferDescription.Width;
		size.y = backBufferDescription.Height;
	}

	void D3D_SwapChainRenderTarget::Resize(ivec2 newSize)
	{
		backBuffer = nullptr;
		renderTargetView = nullptr;

		D3D.SwapChain->ResizeBuffers(0, newSize.x, newSize.y, DXGI_FORMAT_UNKNOWN, 0);
		size = newSize;

		swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &renderTargetView);
	}

	ivec2 D3D_SwapChainRenderTarget::GetSize() const
	{
		return size;
	}

	D3D_RenderTarget::D3D_RenderTarget(ivec2 size)
		: D3D_RenderTarget(size, RenderTargetLDRFormatRGBA)
	{
	}

	D3D_RenderTarget::D3D_RenderTarget(ivec2 size, DXGI_FORMAT format, uint32_t multiSampleCount)
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

	void D3D_RenderTarget::BindResource(uint32_t textureSlot)
	{
		std::array<ID3D11ShaderResourceView*, 1> resourceViews = { GetResourceView() };
		D3D.Context->PSSetShaderResources(textureSlot, static_cast<UINT>(resourceViews.size()), resourceViews.data());
	}

	ivec2 D3D_RenderTarget::GetSize() const
	{
		return ivec2(backBufferDescription.Width, backBufferDescription.Height);
	}

	void D3D_RenderTarget::Resize(ivec2 newSize)
	{
		backBufferDescription.Width = newSize.x;
		backBufferDescription.Height = newSize.y;

		D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);
		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	void D3D_RenderTarget::SetFormat(DXGI_FORMAT format)
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

	uint32_t D3D_RenderTarget::GetMultiSampleCount() const
	{
		return backBufferDescription.SampleDesc.Count;
	}

	ID3D11Resource* D3D_RenderTarget::GetResource() const
	{
		return backBuffer.Get();
	}

	ID3D11ShaderResourceView* D3D_RenderTarget::GetResourceView() const
	{
		return shaderResourceView.Get();
	}

	UniquePtr<uint8_t[]> D3D_RenderTarget::StageAndCopyBackBuffer()
	{
		return StageAndCopyD3DTexture2D(backBuffer.Get(), backBufferDescription);
	}

	D3D_DepthRenderTarget::D3D_DepthRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat)
		: D3D_RenderTarget(size), depthBuffer(size, depthBufferFormat)
	{
	}

	D3D_DepthRenderTarget::D3D_DepthRenderTarget(ivec2 size, DXGI_FORMAT format, DXGI_FORMAT depthBufferFormat, uint32_t multiSampleCount)
		: D3D_RenderTarget(size, format, multiSampleCount), depthBuffer(size, depthBufferFormat, multiSampleCount)
	{
	}

	void D3D_DepthRenderTarget::Bind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { renderTargetView.Get() };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), depthBuffer.GetDepthStencilView());
	}

	void D3D_DepthRenderTarget::UnBind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void D3D_DepthRenderTarget::Clear(const vec4& color)
	{
		D3D_RenderTarget::Clear(color);
		depthBuffer.Clear();
	}

	void D3D_DepthRenderTarget::Resize(ivec2 newSize)
	{
		D3D_RenderTarget::Resize(newSize);
		depthBuffer.Resize(newSize);
	}

	void D3D_DepthRenderTarget::SetMultiSampleCount(uint32_t multiSampleCount)
	{
		backBufferDescription.SampleDesc.Count = multiSampleCount;
		backBufferDescription.SampleDesc.Quality = 0;
		D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);

		renderTargetViewDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_RTV_DIMENSION_TEXTURE2D : D3D11_RTV_DIMENSION_TEXTURE2DMS;
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);

		shaderResourceViewDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);

		depthBuffer.SetMultiSampleCount(multiSampleCount);
	}

	void D3D_DepthRenderTarget::SetMultiSampleCountIfDifferent(uint32_t multiSampleCount)
	{
		if (multiSampleCount != GetMultiSampleCount())
			SetMultiSampleCount(multiSampleCount);
	}

	D3D_DepthBuffer* D3D_DepthRenderTarget::GetDepthBuffer()
	{
		return &depthBuffer;
	}

	D3D_DepthOnlyRenderTarget::D3D_DepthOnlyRenderTarget(ivec2 size, DXGI_FORMAT depthBufferFormat)
		: resourceViewDepthBuffer(size, depthBufferFormat)
	{
	}

	void D3D_DepthOnlyRenderTarget::Bind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), resourceViewDepthBuffer.GetDepthStencilView());
	}

	void D3D_DepthOnlyRenderTarget::UnBind() const
	{
		std::array<ID3D11RenderTargetView*, 1> renderTargetViews = { nullptr };
		D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargetViews.size()), renderTargetViews.data(), nullptr);
	}

	void D3D_DepthOnlyRenderTarget::Clear(const vec4& color)
	{
		resourceViewDepthBuffer.Clear();
	}

	ivec2 D3D_DepthOnlyRenderTarget::GetSize() const
	{
		return resourceViewDepthBuffer.GetSize();
	}

	void D3D_DepthOnlyRenderTarget::Resize(ivec2 newSize)
	{
		resourceViewDepthBuffer.Resize(newSize);
	}

	void D3D_DepthOnlyRenderTarget::BindResource(uint32_t textureSlot)
	{
		std::array<ID3D11ShaderResourceView*, 1> resourceViews = { GetResourceView() };
		D3D.Context->PSSetShaderResources(textureSlot, static_cast<UINT>(resourceViews.size()), resourceViews.data());
	}

	ID3D11ShaderResourceView* D3D_DepthOnlyRenderTarget::GetResourceView() const
	{
		return resourceViewDepthBuffer.GetResourceView();
	}

	D3D_ResourceViewDepthBuffer* D3D_DepthOnlyRenderTarget::GetDepthBuffer()
	{
		return &resourceViewDepthBuffer;
	}
}
