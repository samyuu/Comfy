#include "D3D_Texture2D.h"

namespace Graphics
{
	namespace
	{
		constexpr uint32_t UnboundTextureSlot = 0xFFFFFFFF;
	
		constexpr DXGI_FORMAT GetDxgiFormat(TextureFormat format)
		{
			switch (format)
			{
			case TextureFormat::A8:			return DXGI_FORMAT_R8_UINT;
			case TextureFormat::RGB8:		return DXGI_FORMAT_UNKNOWN;
			case TextureFormat::RGBA8:		return DXGI_FORMAT_R8G8B8A8_UNORM;
			case TextureFormat::RGB5:		return DXGI_FORMAT_UNKNOWN;
			case TextureFormat::RGB5_A1:	return DXGI_FORMAT_UNKNOWN;
			case TextureFormat::RGBA4:		return DXGI_FORMAT_UNKNOWN;
			case TextureFormat::DXT1:		return DXGI_FORMAT_BC1_UNORM;
			case TextureFormat::DXT1a:		return DXGI_FORMAT_BC1_UNORM_SRGB;
			case TextureFormat::DXT3:		return DXGI_FORMAT_BC2_UNORM;
			case TextureFormat::DXT5:		return DXGI_FORMAT_BC3_UNORM;
			case TextureFormat::RGTC1:		return DXGI_FORMAT_BC4_UNORM;
			case TextureFormat::RGTC2:		return DXGI_FORMAT_BC5_UNORM;
			case TextureFormat::L8:			return DXGI_FORMAT_A8_UNORM;
			case TextureFormat::L8A8:		return DXGI_FORMAT_A8P8;
			case TextureFormat::Unknown:	return DXGI_FORMAT_UNKNOWN;
			}

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}
		
		constexpr UINT GetFormatDimensionsAlignmentRestriction(TextureFormat format)
		{
			switch (format)
			{
			case TextureFormat::DXT1:
			case TextureFormat::DXT1a:
			case TextureFormat::DXT3:
			case TextureFormat::DXT5:
			case TextureFormat::RGTC1:
			case TextureFormat::RGTC2:

			default:
				return 0;
			}
		}

		constexpr float GetTxpLodBias(Txp* txp)
		{
			if (txp == nullptr || txp->MipMaps.size() < 1)
				return 0.0f;

			// NOTE: Because somewhat surprisingly Texture2D::SampleLevel also factors in the LOD bias
			return (txp->MipMaps.front()->Format != TextureFormat::RGTC2) ? -1.0f : 0.0f;
		}

		constexpr UINT GetBitsPerPixel(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_R32G32B32A32_TYPELESS:
			case DXGI_FORMAT_R32G32B32A32_FLOAT:
			case DXGI_FORMAT_R32G32B32A32_UINT:
			case DXGI_FORMAT_R32G32B32A32_SINT:
				return 128;

			case DXGI_FORMAT_R32G32B32_TYPELESS:
			case DXGI_FORMAT_R32G32B32_FLOAT:
			case DXGI_FORMAT_R32G32B32_UINT:
			case DXGI_FORMAT_R32G32B32_SINT:
				return 96;

			case DXGI_FORMAT_R16G16B16A16_TYPELESS:
			case DXGI_FORMAT_R16G16B16A16_FLOAT:
			case DXGI_FORMAT_R16G16B16A16_UNORM:
			case DXGI_FORMAT_R16G16B16A16_UINT:
			case DXGI_FORMAT_R16G16B16A16_SNORM:
			case DXGI_FORMAT_R16G16B16A16_SINT:
			case DXGI_FORMAT_R32G32_TYPELESS:
			case DXGI_FORMAT_R32G32_FLOAT:
			case DXGI_FORMAT_R32G32_UINT:
			case DXGI_FORMAT_R32G32_SINT:
			case DXGI_FORMAT_R32G8X24_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
			case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
			case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
			case DXGI_FORMAT_Y416:
			case DXGI_FORMAT_Y210:
			case DXGI_FORMAT_Y216:
				return 64;

			case DXGI_FORMAT_R10G10B10A2_TYPELESS:
			case DXGI_FORMAT_R10G10B10A2_UNORM:
			case DXGI_FORMAT_R10G10B10A2_UINT:
			case DXGI_FORMAT_R11G11B10_FLOAT:
			case DXGI_FORMAT_R8G8B8A8_TYPELESS:
			case DXGI_FORMAT_R8G8B8A8_UNORM:
			case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
			case DXGI_FORMAT_R8G8B8A8_UINT:
			case DXGI_FORMAT_R8G8B8A8_SNORM:
			case DXGI_FORMAT_R8G8B8A8_SINT:
			case DXGI_FORMAT_R16G16_TYPELESS:
			case DXGI_FORMAT_R16G16_FLOAT:
			case DXGI_FORMAT_R16G16_UNORM:
			case DXGI_FORMAT_R16G16_UINT:
			case DXGI_FORMAT_R16G16_SNORM:
			case DXGI_FORMAT_R16G16_SINT:
			case DXGI_FORMAT_R32_TYPELESS:
			case DXGI_FORMAT_D32_FLOAT:
			case DXGI_FORMAT_R32_FLOAT:
			case DXGI_FORMAT_R32_UINT:
			case DXGI_FORMAT_R32_SINT:
			case DXGI_FORMAT_R24G8_TYPELESS:
			case DXGI_FORMAT_D24_UNORM_S8_UINT:
			case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
			case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
			case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
			case DXGI_FORMAT_R8G8_B8G8_UNORM:
			case DXGI_FORMAT_G8R8_G8B8_UNORM:
			case DXGI_FORMAT_B8G8R8A8_UNORM:
			case DXGI_FORMAT_B8G8R8X8_UNORM:
			case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
			case DXGI_FORMAT_B8G8R8A8_TYPELESS:
			case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
			case DXGI_FORMAT_B8G8R8X8_TYPELESS:
			case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
			case DXGI_FORMAT_AYUV:
			case DXGI_FORMAT_Y410:
			case DXGI_FORMAT_YUY2:
				return 32;

			case DXGI_FORMAT_P010:
			case DXGI_FORMAT_P016:
				return 24;

			case DXGI_FORMAT_R8G8_TYPELESS:
			case DXGI_FORMAT_R8G8_UNORM:
			case DXGI_FORMAT_R8G8_UINT:
			case DXGI_FORMAT_R8G8_SNORM:
			case DXGI_FORMAT_R8G8_SINT:
			case DXGI_FORMAT_R16_TYPELESS:
			case DXGI_FORMAT_R16_FLOAT:
			case DXGI_FORMAT_D16_UNORM:
			case DXGI_FORMAT_R16_UNORM:
			case DXGI_FORMAT_R16_UINT:
			case DXGI_FORMAT_R16_SNORM:
			case DXGI_FORMAT_R16_SINT:
			case DXGI_FORMAT_B5G6R5_UNORM:
			case DXGI_FORMAT_B5G5R5A1_UNORM:
			case DXGI_FORMAT_A8P8:
			case DXGI_FORMAT_B4G4R4A4_UNORM:
				return 16;

			case DXGI_FORMAT_NV12:
			case DXGI_FORMAT_420_OPAQUE:
			case DXGI_FORMAT_NV11:
				return 12;

			case DXGI_FORMAT_R8_TYPELESS:
			case DXGI_FORMAT_R8_UNORM:
			case DXGI_FORMAT_R8_UINT:
			case DXGI_FORMAT_R8_SNORM:
			case DXGI_FORMAT_R8_SINT:
			case DXGI_FORMAT_A8_UNORM:
			case DXGI_FORMAT_AI44:
			case DXGI_FORMAT_IA44:
			case DXGI_FORMAT_P8:
				return 8;

			case DXGI_FORMAT_R1_UNORM:
				return 1;

			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
				return 4;

			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return 8;

			default:
				return 0;
			}
		}

		constexpr bool UsesBlockCompression(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_BC1_TYPELESS:
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
			case DXGI_FORMAT_BC2_TYPELESS:
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
			case DXGI_FORMAT_BC3_TYPELESS:
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
			case DXGI_FORMAT_BC4_TYPELESS:
			case DXGI_FORMAT_BC4_UNORM:
			case DXGI_FORMAT_BC4_SNORM:
			case DXGI_FORMAT_BC5_TYPELESS:
			case DXGI_FORMAT_BC5_UNORM:
			case DXGI_FORMAT_BC5_SNORM:
			case DXGI_FORMAT_BC6H_TYPELESS:
			case DXGI_FORMAT_BC6H_UF16:
			case DXGI_FORMAT_BC6H_SF16:
			case DXGI_FORMAT_BC7_TYPELESS:
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return true;

			default:
				return false;
			}
		}

		UINT PadTextureDimension(const UINT dimension, const UINT alignment)
		{
			const UINT padding = ((dimension + (alignment - 1)) & ~(alignment - 1)) - dimension;
			return dimension + padding;
		}

		void PadTextureDimensions(UINT& width, UINT& height, const UINT alignment)
		{
			width = PadTextureDimension(width, alignment);
			height = PadTextureDimension(height, alignment);
		}
	}

	D3D_Texture2D::D3D_Texture2D(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, float lodBias)
		: lastBoundSlot(UnboundTextureSlot), textureFormat(TextureFormat::Unknown)
	{
		constexpr vec4 transparentBorderColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);

		samplerDescription.Filter = filter;
		samplerDescription.AddressU = addressMode;
		samplerDescription.AddressV = addressMode;
		samplerDescription.AddressW = addressMode;
		samplerDescription.MipLODBias = lodBias;
		samplerDescription.MaxAnisotropy = 0;
		samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDescription.BorderColor[0] = transparentBorderColor[0];
		samplerDescription.BorderColor[1] = transparentBorderColor[1];
		samplerDescription.BorderColor[2] = transparentBorderColor[2];
		samplerDescription.BorderColor[3] = transparentBorderColor[3];
		samplerDescription.MinLOD = 0.0f;
		samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

		D3D.Device->CreateSamplerState(&samplerDescription, &samplerState);
	}

	void D3D_Texture2D::Bind(uint32_t textureSlot) const
	{
		assert(textureSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
		lastBoundSlot = textureSlot;

		constexpr UINT textureCount = 1;

		std::array<ID3D11SamplerState*, textureCount> samplerStates = { samplerState.Get() };
		std::array<ID3D11ShaderResourceView*, textureCount> resourceViews = { resourceView.Get() };

		D3D.Context->PSSetSamplers(textureSlot, textureCount, samplerStates.data());
		D3D.Context->PSSetShaderResources(textureSlot, textureCount, resourceViews.data());
	}
	
	void D3D_Texture2D::UnBind() const
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

	TextureFormat D3D_Texture2D::GetTextureFormat() const
	{
		return textureFormat;
	}

	void* D3D_Texture2D::GetVoidTexture() const
	{
		return resourceView.Get();
	}

	D3D_ImmutableTexture2D::D3D_ImmutableTexture2D(Txp* txp)
		: D3D_Texture2D(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, GetTxpLodBias(txp))
	{
		assert(txp != nullptr && txp->MipMaps.size() > 0);
		auto& baseMipMap = txp->MipMaps.front();

		textureFormat = baseMipMap->Format;
		textureDescription.Width = baseMipMap->Width;
		textureDescription.Height = baseMipMap->Height;
		textureDescription.MipLevels = static_cast<UINT>(txp->MipMaps.size());
		textureDescription.ArraySize = 1;
		textureDescription.Format = GetDxgiFormat(baseMipMap->Format);
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = 0;

		constexpr UINT blockCompressionAlignment = 4;
		const bool usesBlockCompression = UsesBlockCompression(textureDescription.Format);

		if (usesBlockCompression)
			PadTextureDimensions(textureDescription.Width, textureDescription.Height, blockCompressionAlignment);

		const UINT bitsPerPixel = GetBitsPerPixel(textureDescription.Format);
		
		// NOTE: This doesn't seem to be defined anywhere but this should cover even the largest supported textures
		constexpr size_t maxMipMaps = 16;
		D3D11_SUBRESOURCE_DATA initialResourceData[maxMipMaps];
		
		for (size_t i = 0; i < txp->MipMaps.size(); i++)
		{
			const auto& mipMap = txp->MipMaps[i];
			D3D11_SUBRESOURCE_DATA& resource = initialResourceData[i];

			resource.pSysMem = mipMap->DataPointer != nullptr ? mipMap->DataPointer : mipMap->Data.data();

			// TODO: Not sure if this is entirely accurate or reliable
			resource.SysMemPitch = (usesBlockCompression) ? 
				((PadTextureDimension(mipMap->Width, blockCompressionAlignment) * bitsPerPixel) / 2) :
				((mipMap->Width * bitsPerPixel + 7) / 8);

			resource.SysMemSlicePitch = 0;
		}
		
		D3D.Device->CreateTexture2D(&textureDescription, initialResourceData, &texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;
		resourceViewDescription.Format = textureDescription.Format;
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDescription.Texture2D.MostDetailedMip = 0;
		resourceViewDescription.Texture2D.MipLevels = textureDescription.MipLevels;

		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}
	
	D3D_ImmutableTexture2D::D3D_ImmutableTexture2D(ivec2 size, const void* rgbaBuffer)
		: D3D_Texture2D(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER, 0.0f)
	{
		textureFormat = TextureFormat::RGBA8;
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

		D3D11_SUBRESOURCE_DATA initialResourceData = { rgbaBuffer, textureDescription.Width * rgbaBytesPerPixel, 0 };
		D3D.Device->CreateTexture2D(&textureDescription, &initialResourceData, &texture);

		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;
		resourceViewDescription.Format = textureDescription.Format;
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		resourceViewDescription.Texture2D.MostDetailedMip = 0;
		resourceViewDescription.Texture2D.MipLevels = textureDescription.MipLevels;

		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}
}
