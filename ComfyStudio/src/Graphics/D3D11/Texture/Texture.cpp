#include "Texture.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth3D/LightParam/IBLParameters.h"

namespace Comfy::Graphics::D3D11
{
	namespace
	{
		// NOTE: This doesn't seem to be defined anywhere but this should cover even the largest supported textures
		constexpr size_t MaxMipMaps = 16;
		constexpr size_t CubeFaceCount = 6;

		constexpr u32 BlockCompressionAlignment = 4;
		constexpr u32 UnboundTextureSlot = 0xFFFFFFFF;

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

		constexpr DXGI_FORMAT GetDxgiFormat(LightMapFormat format)
		{
			switch (format)
			{
			case LightMapFormat::RGBA8_CUBE:	return DXGI_FORMAT_R8G8B8A8_UNORM;
			case LightMapFormat::RGBA16F_CUBE:	return DXGI_FORMAT_R16G16B16A16_FLOAT;
			case LightMapFormat::RGBA32F_CUBE:	return DXGI_FORMAT_R32G32B32A32_FLOAT;
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

		constexpr UINT PadTextureDimension(const UINT dimension, const UINT alignment)
		{
			const UINT padding = ((dimension + (alignment - 1)) & ~(alignment - 1)) - dimension;
			return dimension + padding;
		}

		constexpr void PadTextureDimensions(UINT& width, UINT& height, const UINT alignment)
		{
			width = PadTextureDimension(width, alignment);
			height = PadTextureDimension(height, alignment);
		}

		constexpr UINT GetMemoryPitch(const ivec2& size, UINT bitsPerPixel, bool usesBlockCompression = false)
		{
			if (usesBlockCompression)
			{
				// TODO: Not sure if this is entirely accurate and or reliable
				return ((PadTextureDimension(size.x, BlockCompressionAlignment) * bitsPerPixel) / 2);
			}
			else
			{
				return ((size.x * bitsPerPixel + 7) / 8);
			}
		}

		void PadRGBToRGBA(const ivec2 size, const u8* inputRGB, u32* outputRGBA)
		{
			const u8* currentRGBPixel = inputRGB;

			for (i32 i = 0; i < (size.x * size.y); i++)
			{
				const u8 r = currentRGBPixel[0];
				const u8 g = currentRGBPixel[1];
				const u8 b = currentRGBPixel[2];
				const u8 a = std::numeric_limits<u8>::max();
				currentRGBPixel += 3;

				const u32 rgba = (r << 0) | (g << 8) | (b << 16) | (a << 24);
				outputRGBA[i] = rgba;
			}
		}

		D3D11_SUBRESOURCE_DATA CreateMipMapSubresourceData(const TexMipMap& mipMap, bool usesBlockCompression, UINT bitsPerPixel)
		{
			D3D11_SUBRESOURCE_DATA resource;
			resource.pSysMem = mipMap.Data.get();
			resource.SysMemPitch = GetMemoryPitch(mipMap.Size, bitsPerPixel, usesBlockCompression);
			resource.SysMemSlicePitch = 0;
			return resource;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC CreateTextureResourceViewDescription(const D3D11_TEXTURE2D_DESC& textureDescription)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription = {};
			resourceViewDescription.Format = textureDescription.Format;

			if (textureDescription.ArraySize <= 1)
			{
				resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				resourceViewDescription.Texture2D.MostDetailedMip = 0;
				resourceViewDescription.Texture2D.MipLevels = textureDescription.MipLevels;
			}
			else
			{
				resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
				resourceViewDescription.TextureCube.MostDetailedMip = 0;
				resourceViewDescription.TextureCube.MipLevels = textureDescription.MipLevels;
			}

			return resourceViewDescription;
		}

		constexpr std::array<D3D11_TEXTURECUBE_FACE, CubeFaceCount> TexCubeFaceIndices =
		{
			D3D11_TEXTURECUBE_FACE_POSITIVE_X,
			D3D11_TEXTURECUBE_FACE_NEGATIVE_X,
			D3D11_TEXTURECUBE_FACE_POSITIVE_Y,
			D3D11_TEXTURECUBE_FACE_NEGATIVE_Y,
			D3D11_TEXTURECUBE_FACE_NEGATIVE_Z,
			D3D11_TEXTURECUBE_FACE_POSITIVE_Z,
		};

		constexpr std::array<D3D11_TEXTURECUBE_FACE, CubeFaceCount> LightMapCubeFaceIndices =
		{
			D3D11_TEXTURECUBE_FACE_POSITIVE_X,
			D3D11_TEXTURECUBE_FACE_NEGATIVE_X,
			D3D11_TEXTURECUBE_FACE_POSITIVE_Y,
			D3D11_TEXTURECUBE_FACE_NEGATIVE_Y,
			D3D11_TEXTURECUBE_FACE_POSITIVE_Z,
			D3D11_TEXTURECUBE_FACE_NEGATIVE_Z,
		};
	}

	TextureResource::TextureResource()
		: lastBoundSlot(UnboundTextureSlot), textureFormat(TextureFormat::Unknown)
	{
	}

	TextureResource::~TextureResource()
	{
		D3D.EnsureDeviceObjectLifetimeUntilRendering(resourceView.Get());
	}

	void TextureResource::Bind(u32 textureSlot) const
	{
		assert(textureSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		lastBoundSlot = textureSlot;

		constexpr UINT textureCount = 1;
		std::array<ID3D11ShaderResourceView*, textureCount> resourceViews = { resourceView.Get() };

		D3D.Context->PSSetShaderResources(textureSlot, textureCount, resourceViews.data());
	}

	void TextureResource::UnBind() const
	{
		assert(lastBoundSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);

		constexpr UINT textureCount = 1;
		std::array<ID3D11ShaderResourceView*, textureCount> resourceViews = { nullptr };

		D3D.Context->PSSetShaderResources(lastBoundSlot, textureCount, resourceViews.data());

		lastBoundSlot = UnboundTextureSlot;
	}

	ivec2 TextureResource::GetSize() const
	{
		return ivec2(textureDescription.Width, textureDescription.Height);
	}

	ID3D11Texture2D* TextureResource::GetTexture()
	{
		return texture.Get();
	}

	TextureFormat TextureResource::GetTextureFormat() const
	{
		return textureFormat;
	}

	ID3D11ShaderResourceView* TextureResource::GetResourceView() const
	{
		return resourceView.Get();
	}

	Texture1D::Texture1D(i32 width, const void* pixelData, DXGI_FORMAT format)
	{
		textureDescription.Width = width;
		textureDescription.MipLevels = 1;
		textureDescription.ArraySize = 1;
		textureDescription.Format = format;
		textureDescription.Usage = D3D11_USAGE_DYNAMIC;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		textureDescription.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initialResourceData = { pixelData, 0, 0 };
		D3D.Device->CreateTexture1D(&textureDescription, (pixelData == nullptr) ? nullptr : &initialResourceData, &texture);

		resourceViewDescription = {};
		resourceViewDescription.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE1D;
		resourceViewDescription.Texture1D.MostDetailedMip = 0;
		resourceViewDescription.Texture1D.MipLevels = textureDescription.MipLevels;
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}

	void Texture1D::UploadData(size_t dataSize, const void* pixelData)
	{
		// assert(dataSize ...);

		D3D11_MAPPED_SUBRESOURCE mappedTexture;
		D3D.Context->Map(texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedTexture);
		{
			std::memcpy(mappedTexture.pData, pixelData, dataSize);
		}
		D3D.Context->Unmap(texture.Get(), 0);
	}

	ID3D11ShaderResourceView* Texture1D::GetResourceView() const
	{
		return resourceView.Get();
	}

	Texture2D::Texture2D(const Tex& tex)
	{
		assert(tex.MipMapsArray.size() == GetArraySize() && tex.MipMapsArray.front().size() > 0 && tex.GetSignature() == TxpSig::Texture2D);

		auto& mipMaps = tex.MipMapsArray.front();
		auto& baseMipMap = mipMaps.front();

		textureFormat = baseMipMap.Format;
		textureDescription.Width = baseMipMap.Size.x;
		textureDescription.Height = baseMipMap.Size.y;
		textureDescription.MipLevels = static_cast<UINT>(mipMaps.size());
		textureDescription.ArraySize = static_cast<UINT>(tex.MipMapsArray.size());
		textureDescription.Format = GetDxgiFormat(baseMipMap.Format);
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = 0;

		std::array<D3D11_SUBRESOURCE_DATA, MaxMipMaps> initialResourceData;

		// NOTE: Natively unsupported 24-bit format so it needs to be padded first
		if (baseMipMap.Format == TextureFormat::RGB8)
		{
			textureDescription.Format = GetDxgiFormat(TextureFormat::RGBA8);
			std::array<std::unique_ptr<u32[]>, MaxMipMaps> rgbaBuffers;

			for (size_t i = 0; i < mipMaps.size(); i++)
			{
				auto& resource = initialResourceData[i];
				auto& mipMap = mipMaps[i];

				rgbaBuffers[i] = std::make_unique<u32[]>(mipMap.Size.x * mipMap.Size.y);
				PadRGBToRGBA(mipMap.Size, mipMap.Data.get(), rgbaBuffers[i].get());

				resource.pSysMem = rgbaBuffers[i].get();
				resource.SysMemPitch = GetMemoryPitch(mipMap.Size, GetBitsPerPixel(textureDescription.Format), false);
				resource.SysMemSlicePitch = 0;
			}

			D3D.Device->CreateTexture2D(&textureDescription, initialResourceData.data(), &texture);
		}
		else
		{
			const bool usesBlockCompression = UsesBlockCompression(textureDescription.Format);
			const UINT bitsPerPixel = GetBitsPerPixel(textureDescription.Format);

			if (usesBlockCompression)
				PadTextureDimensions(textureDescription.Width, textureDescription.Height, BlockCompressionAlignment);

			for (size_t i = 0; i < mipMaps.size(); i++)
				initialResourceData[i] = CreateMipMapSubresourceData(mipMaps[i], usesBlockCompression, bitsPerPixel);

			D3D.Device->CreateTexture2D(&textureDescription, initialResourceData.data(), &texture);
		}

		resourceViewDescription = CreateTextureResourceViewDescription(textureDescription);
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}

	Texture2D::Texture2D(ivec2 size, const u32* rgbaBuffer)
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

		D3D11_SUBRESOURCE_DATA initialResourceData = { rgbaBuffer, GetMemoryPitch(size, GetBitsPerPixel(textureDescription.Format)), 0 };
		D3D.Device->CreateTexture2D(&textureDescription, &initialResourceData, &texture);

		resourceViewDescription = CreateTextureResourceViewDescription(textureDescription);
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}

	u32 Texture2D::GetArraySize() const
	{
		return 1;
	}

	CubeMap::CubeMap(const Tex& tex)
	{
		assert(tex.MipMapsArray.size() == GetArraySize() && tex.MipMapsArray.front().size() > 0 && tex.GetSignature() == TxpSig::CubeMap);

		auto& mipMaps = tex.MipMapsArray.front();
		auto& baseMipMap = mipMaps.front();

		textureFormat = baseMipMap.Format;
		textureDescription.Width = baseMipMap.Size.x;
		textureDescription.Height = baseMipMap.Size.y;
		textureDescription.MipLevels = static_cast<UINT>(mipMaps.size());
		textureDescription.ArraySize = static_cast<UINT>(tex.MipMapsArray.size());
		textureDescription.Format = GetDxgiFormat(baseMipMap.Format);
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		const bool usesBlockCompression = UsesBlockCompression(textureDescription.Format);
		const UINT bitsPerPixel = GetBitsPerPixel(textureDescription.Format);

		if (usesBlockCompression)
			PadTextureDimensions(textureDescription.Width, textureDescription.Height, BlockCompressionAlignment);

		std::array<D3D11_SUBRESOURCE_DATA, CubeFaceCount * MaxMipMaps> initialResourceData;

		for (u32 faceIndex = 0; faceIndex < CubeFaceCount; faceIndex++)
		{
			for (u32 mipIndex = 0; mipIndex < textureDescription.MipLevels; mipIndex++)
				initialResourceData[TexCubeFaceIndices[faceIndex] * textureDescription.MipLevels + mipIndex] = CreateMipMapSubresourceData(tex.MipMapsArray[faceIndex][mipIndex], usesBlockCompression, bitsPerPixel);
		}

		D3D.Device->CreateTexture2D(&textureDescription, initialResourceData.data(), &texture);

		resourceViewDescription = CreateTextureResourceViewDescription(textureDescription);
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}

	CubeMap::CubeMap(const LightMapIBL& lightMap)
	{
		u32 mipMapLevels = 1;
		for (u32 i = 0; i < lightMap.DataPointers[0].size(); i++)
		{
			if (lightMap.DataPointers[0][i] == nullptr)
				break;
			mipMapLevels = i + 1;
		}

		textureFormat = TextureFormat::Unknown;
		textureDescription.Width = lightMap.Size.x;
		textureDescription.Height = lightMap.Size.y;
		textureDescription.MipLevels = mipMapLevels;
		textureDescription.ArraySize = CubeFaceCount;
		textureDescription.Format = GetDxgiFormat(lightMap.Format);
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		const UINT bitsPerPixel = GetBitsPerPixel(textureDescription.Format);

		std::array<D3D11_SUBRESOURCE_DATA, CubeFaceCount * MaxMipMaps> initialResourceData = {};

		for (u32 faceIndex = 0; faceIndex < CubeFaceCount; faceIndex++)
		{
			for (u32 mipIndex = 0; mipIndex < textureDescription.MipLevels; mipIndex++)
			{
				const ivec2 mipMapSize = (lightMap.Size >> static_cast<i32>(mipIndex));
				initialResourceData[LightMapCubeFaceIndices[faceIndex] * textureDescription.MipLevels + mipIndex] = { lightMap.DataPointers[faceIndex][mipIndex], GetMemoryPitch(mipMapSize, bitsPerPixel, false), 0 };
			}
		}

		D3D.Device->CreateTexture2D(&textureDescription, initialResourceData.data(), &texture);

		resourceViewDescription = CreateTextureResourceViewDescription(textureDescription);
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}

	u32 CubeMap::GetArraySize() const
	{
		return CubeFaceCount;
	}
}
