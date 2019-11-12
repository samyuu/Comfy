#include "D3D_DepthBuffer.h"

namespace Graphics
{
	D3D_DepthBuffer::D3D_DepthBuffer(ivec2 size, DXGI_FORMAT format)
	{
		textureDescription.Width = size.x;
		textureDescription.Height = size.y;
		textureDescription.MipLevels = 1;
		textureDescription.ArraySize = 1;
		textureDescription.Format = format;
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_DEFAULT;
		textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = 0;

		D3D.Device->CreateTexture2D(&textureDescription, nullptr, &depthTexture);

		depthStencilDescription.Format = format;
		depthStencilDescription.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilDescription.Flags = 0;
		depthStencilDescription.Texture2D.MipSlice = 0;

		D3D.Device->CreateDepthStencilView(depthTexture.Get(), &depthStencilDescription, &depthStencilView);
	}

	void D3D_DepthBuffer::Clear(float value)
	{
		D3D.Context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, value, 0);
	}

	void D3D_DepthBuffer::Resize(ivec2 newSize)
	{
		textureDescription.Width = newSize.x;
		textureDescription.Height = newSize.y;

		// TODO: I hope this doesn't leak any memory to reasign a ComPtr like this (?)
		D3D.Device->CreateTexture2D(&textureDescription, nullptr, &depthTexture);
		D3D.Device->CreateDepthStencilView(depthTexture.Get(), &depthStencilDescription, &depthStencilView);
	}

	ivec2 D3D_DepthBuffer::GetSize() const
	{
		return ivec2(textureDescription.Width, textureDescription.Height);
	}

	ID3D11DepthStencilView* D3D_DepthBuffer::GetDepthStencilView()
	{
		return depthStencilView.Get();
	}
}
