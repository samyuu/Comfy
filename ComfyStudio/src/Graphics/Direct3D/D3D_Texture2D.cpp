#include "D3D_Texture2D.h"

namespace Graphics
{
	constexpr uint32_t UnboundTextureSlot = 0xFFFFFFFF;
	constexpr vec4 DebugTextureBorderColor = vec4(1.0f, 0.0f, 1.0f, 1.0f);

	D3D_Texture2D::D3D_Texture2D(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode)
		: lastBoundSlot(UnboundTextureSlot)
	{
		samplerDescription.Filter = filter;
		samplerDescription.AddressU = addressMode;
		samplerDescription.AddressV = addressMode;
		samplerDescription.AddressW = addressMode;
		samplerDescription.MipLODBias = 0.0f;
		samplerDescription.MaxAnisotropy = 0;
		samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDescription.BorderColor[0] = DebugTextureBorderColor[0];
		samplerDescription.BorderColor[1] = DebugTextureBorderColor[1];
		samplerDescription.BorderColor[2] = DebugTextureBorderColor[2];
		samplerDescription.BorderColor[3] = DebugTextureBorderColor[3];
		samplerDescription.MinLOD = 0;
		samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

		D3D.Device->CreateSamplerState(&samplerDescription, &samplerState);
	}

	void D3D_Texture2D::Bind(uint32_t textureSlot)
	{
		assert(textureSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
		lastBoundSlot = textureSlot;

		constexpr UINT startSlot = 0;
		constexpr UINT textureCount = 1;

		std::array<ID3D11SamplerState*, textureCount> samplerStates = { samplerState.Get() };
		std::array<ID3D11ShaderResourceView*, textureCount> resourceViews = { resourceView.Get() };

		D3D.Context->PSSetSamplers(startSlot, textureCount, samplerStates.data());
		D3D.Context->PSSetShaderResources(startSlot, textureCount, resourceViews.data());
	}
	
	void D3D_Texture2D::UnBind()
	{
		assert(lastBoundSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

		constexpr UINT textureCount = 1;
		
		std::array<ID3D11SamplerState*, textureCount> samplerStates = { nullptr };
		std::array<ID3D11ShaderResourceView*, textureCount> resourceViews = { nullptr };

		D3D.Context->PSSetSamplers(lastBoundSlot, textureCount, samplerStates.data());
		D3D.Context->PSSetShaderResources(lastBoundSlot, textureCount, resourceViews.data());

		lastBoundSlot = UnboundTextureSlot;
	}
	
	ivec2 D3D_Texture2D::GetSize() const
	{
		return ivec2(textureDescription.Width, textureDescription.Height);
	}

	ID3D11Texture2D* D3D_Texture2D::GetTexture()
	{
		return texture.Get();
	}

	D3D_ImmutableTexture2D::D3D_ImmutableTexture2D(Txp* txp)
		: D3D_Texture2D(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP)
	{
		// TODO:
		assert(false);

		// textureDescription.xxx
		// D3D.Device->CreateTexture2D(&textureDescription, &initialResourceData, &texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;
		resourceViewDescription.Format = textureDescription.Format;
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDescription.Texture2D.MostDetailedMip = 0;
		resourceViewDescription.Texture2D.MipLevels = textureDescription.MipLevels;

		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}
	
	D3D_ImmutableTexture2D::D3D_ImmutableTexture2D(ivec2 size, const void* rgbaBuffer)
		: D3D_Texture2D(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP)
	{
		textureDescription.Width = size.x;
		textureDescription.Height = size.y;
		textureDescription.MipLevels = 1;
		textureDescription.ArraySize = 1;
		textureDescription.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = 0;

		constexpr UINT rgbaBytesPerPixel = 4;

		D3D11_SUBRESOURCE_DATA initialResourceData = { rgbaBuffer, size.x * rgbaBytesPerPixel, 0 };
		D3D.Device->CreateTexture2D(&textureDescription, &initialResourceData, &texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;
		resourceViewDescription.Format = textureDescription.Format;
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDescription.Texture2D.MostDetailedMip = 0;
		resourceViewDescription.Texture2D.MipLevels = textureDescription.MipLevels;

		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}
}
