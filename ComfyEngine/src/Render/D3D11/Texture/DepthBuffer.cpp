#include "DepthBuffer.h"

namespace Comfy::Render::D3D11
{
	namespace
	{
		constexpr DXGI_FORMAT GetDepthTextureFormat(DXGI_FORMAT baseFormat)
		{
			if (baseFormat == DXGI_FORMAT_D32_FLOAT)
				return DXGI_FORMAT_R32_TYPELESS;

			if (baseFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
				return DXGI_FORMAT_R24G8_TYPELESS;

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}

		constexpr DXGI_FORMAT GetDepthStencilFormat(DXGI_FORMAT baseFormat)
		{
			if (baseFormat == DXGI_FORMAT_D32_FLOAT)
				return DXGI_FORMAT_D32_FLOAT;

			if (baseFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
				return DXGI_FORMAT_D24_UNORM_S8_UINT;

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}

		constexpr DXGI_FORMAT GetDepthResourceViewFormat(DXGI_FORMAT baseFormat)
		{
			if (baseFormat == DXGI_FORMAT_D32_FLOAT)
				return DXGI_FORMAT_R32_FLOAT;

			if (baseFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	DepthBuffer::DepthBuffer(ivec2 size, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat, D3D11_BIND_FLAG bindFlags, u32 multiSampleCount)
	{
		textureDescription.Width = size.x;
		textureDescription.Height = size.y;
		textureDescription.MipLevels = 1;
		textureDescription.ArraySize = 1;
		textureDescription.Format = textureFormat;
		textureDescription.SampleDesc.Count = multiSampleCount;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_DEFAULT;
		textureDescription.BindFlags = D3D11_BIND_DEPTH_STENCIL | bindFlags;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = 0;

		D3D.Device->CreateTexture2D(&textureDescription, nullptr, &depthTexture);

		depthStencilDescription.Format = depthFormat;
		depthStencilDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
		depthStencilDescription.Flags = 0;
		depthStencilDescription.Texture2D.MipSlice = 0;

		D3D.Device->CreateDepthStencilView(depthTexture.Get(), &depthStencilDescription, &depthStencilView);
	}

	DepthBuffer::DepthBuffer(ivec2 size, DXGI_FORMAT format, u32 multiSampleCount)
		: DepthBuffer(size, format, format, D3D11_BIND_FLAG {}, multiSampleCount)
	{
	}

	void DepthBuffer::Clear(float value)
	{
		if (depthStencilDescription.Format == DXGI_FORMAT_D24_UNORM_S8_UINT)
			D3D.Context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, value, 0xFF);
		else
			D3D.Context->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, value, 0x00);
	}

	void DepthBuffer::Resize(ivec2 newSize)
	{
		textureDescription.Width = newSize.x;
		textureDescription.Height = newSize.y;

		D3D.Device->CreateTexture2D(&textureDescription, nullptr, &depthTexture);
		D3D.Device->CreateDepthStencilView(depthTexture.Get(), &depthStencilDescription, &depthStencilView);
	}

	void DepthBuffer::SetMultiSampleCount(u32 multiSampleCount)
	{
		textureDescription.SampleDesc.Count = multiSampleCount;
		textureDescription.SampleDesc.Quality = 0;
		if (FAILED(D3D.Device->CreateTexture2D(&textureDescription, nullptr, &depthTexture)))
		{
			textureDescription.SampleDesc.Count = multiSampleCount = 1;
			D3D.Device->CreateTexture2D(&textureDescription, nullptr, &depthTexture);
		}

		depthStencilDescription.ViewDimension = (multiSampleCount == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
		D3D.Device->CreateDepthStencilView(depthTexture.Get(), &depthStencilDescription, &depthStencilView);
	}

	u32 DepthBuffer::GetMultiSampleCount() const
	{
		return textureDescription.SampleDesc.Count;
	}

	ivec2 DepthBuffer::GetSize() const
	{
		return ivec2(textureDescription.Width, textureDescription.Height);
	}

	ID3D11DepthStencilView* DepthBuffer::GetDepthStencilView() const
	{
		return depthStencilView.Get();
	}

	ResourceViewDepthBuffer::ResourceViewDepthBuffer(ivec2 size, DXGI_FORMAT format)
		: DepthBuffer(size, GetDepthTextureFormat(format), GetDepthStencilFormat(format), D3D11_BIND_SHADER_RESOURCE, 1)
	{
		shaderResourceViewDescription.Format = GetDepthResourceViewFormat(format);
		shaderResourceViewDescription.ViewDimension = (textureDescription.SampleDesc.Count == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
		shaderResourceViewDescription.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDescription.Texture2D.MipLevels = 1;

		D3D.Device->CreateShaderResourceView(depthTexture.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	void ResourceViewDepthBuffer::Resize(ivec2 newSize)
	{
		DepthBuffer::Resize(newSize);
		D3D.Device->CreateShaderResourceView(depthTexture.Get(), &shaderResourceViewDescription, &shaderResourceView);
	}

	ID3D11ShaderResourceView* ResourceViewDepthBuffer::GetResourceView() const
	{
		return shaderResourceView.Get();
	}
}
