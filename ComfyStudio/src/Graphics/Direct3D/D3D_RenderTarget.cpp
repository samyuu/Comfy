#include "D3D_RenderTarget.h"

namespace Graphics
{
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
		backBufferDescription.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
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

		// TODO: Do all of these need to be recreated? - Should the ComPtrs be explicitly reset before?
		D3D.Device->CreateTexture2D(&backBufferDescription, nullptr, &backBuffer);
		D3D.Device->CreateRenderTargetView(backBuffer.Get(), &renderTargetViewDescription, &renderTargetView);
		D3D.Device->CreateShaderResourceView(backBuffer.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	uint32_t D3D_RenderTarget::GetMultiSampleCount() const
	{
		return backBufferDescription.SampleDesc.Count;
	}

	ID3D11ShaderResourceView* D3D_RenderTarget::GetResourceView() const
	{
		return shaderResourceView.Get();
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
		D3D_RenderTarget::UnBind();
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
}
