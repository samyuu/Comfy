#include "Texture.h"
#include "RenderTarget.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth3D/LightParam/IBLParameters.h"
#include "Graphics/Utilities/TextureCompression.h"
#include <DirectXTex.h>

namespace Comfy::Render::D3D11
{
	using namespace Graphics;

	namespace
	{
		// NOTE: This doesn't seem to be defined anywhere but this should cover even the largest supported textures
		constexpr size_t MaxMipMaps = 16;
		constexpr size_t CubeFaceCount = 6;

		constexpr u32 BlockCompressionAlignment = 4;
		constexpr u32 UnboundTextureSlot = 0xFFFFFFFF;

		constexpr DXGI_FORMAT GetDXGIFormat(TextureFormat format)
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

		constexpr DXGI_FORMAT GetDXGIFormat(LightMapFormat format)
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

		constexpr size_t PadTextureDimension(const size_t dimension, const size_t alignment)
		{
			const size_t padding = ((dimension + (alignment - 1)) & ~(alignment - 1)) - dimension;
			return dimension + padding;
		}

		constexpr void PadTextureDimensions(UINT& width, UINT& height, const size_t alignment)
		{
			width = static_cast<UINT>(PadTextureDimension(width, alignment));
			height = static_cast<UINT>(PadTextureDimension(height, alignment));
		}

		constexpr size_t GetMemoryPitch(const ivec2& size, size_t bitsPerPixel, bool usesBlockCompression = false)
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

		D3D11_SUBRESOURCE_DATA CreateMipMapSubresourceData(const TexMipMap& mipMap, bool usesBlockCompression, size_t bitsPerPixel)
		{
			D3D11_SUBRESOURCE_DATA resource;
			resource.pSysMem = mipMap.Data.get();
			resource.SysMemPitch = static_cast<UINT>(GetMemoryPitch(mipMap.Size, bitsPerPixel, usesBlockCompression));
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
		textureDescription.Format = GetDXGIFormat(baseMipMap.Format);
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
			textureDescription.Format = GetDXGIFormat(TextureFormat::RGBA8);
			std::array<std::unique_ptr<u8[]>, MaxMipMaps> rgbaBuffers;

			for (size_t i = 0; i < mipMaps.size(); i++)
			{
				auto& resource = initialResourceData[i];
				auto& mipMap = mipMaps[i];

				const auto rgbaByteSize = Utilities::TextureFormatByteSize(mipMap.Size, TextureFormat::RGBA8);
				rgbaBuffers[i] = std::make_unique<u8[]>(rgbaByteSize);

				Utilities::ConvertRGBToRGBA(mipMap.Size, mipMap.Data.get(), mipMap.DataSize, rgbaBuffers[i].get(), rgbaByteSize);

				resource.pSysMem = rgbaBuffers[i].get();
				resource.SysMemPitch = static_cast<UINT>(GetMemoryPitch(mipMap.Size, ::DirectX::BitsPerPixel(textureDescription.Format), false));
				resource.SysMemSlicePitch = 0;
			}

			D3D.Device->CreateTexture2D(&textureDescription, initialResourceData.data(), &texture);
		}
		else
		{
			const bool usesBlockCompression = ::DirectX::IsCompressed(textureDescription.Format);
			const size_t bitsPerPixel = ::DirectX::BitsPerPixel(textureDescription.Format);

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

		D3D11_SUBRESOURCE_DATA initialResourceData = { rgbaBuffer, static_cast<UINT>(GetMemoryPitch(size, ::DirectX::BitsPerPixel(textureDescription.Format))), 0 };
		D3D.Device->CreateTexture2D(&textureDescription, &initialResourceData, &texture);

		resourceViewDescription = CreateTextureResourceViewDescription(textureDescription);
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
	}

	Texture2D::Texture2D(const RenderTarget& sourceRenderTargetToCopy)
	{
		CreateCopy(sourceRenderTargetToCopy);
	}

	u32 Texture2D::GetArraySize() const
	{
		return 1;
	}

	void Texture2D::CreateCopy(const RenderTarget& sourceRenderTargetToCopy)
	{
		textureFormat = TextureFormat::RGBA8;
		textureDescription = sourceRenderTargetToCopy.GetBackBufferDescription();

		if (texture == nullptr)
			D3D.Device->CreateTexture2D(&textureDescription, nullptr, &texture);

		D3D.Context->CopyResource(texture.Get(), sourceRenderTargetToCopy.GetResource());

		resourceViewDescription = CreateTextureResourceViewDescription(textureDescription);
		D3D.Device->CreateShaderResourceView(texture.Get(), &resourceViewDescription, &resourceView);
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
		textureDescription.Format = GetDXGIFormat(baseMipMap.Format);
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		const bool usesBlockCompression = ::DirectX::IsCompressed(textureDescription.Format);
		const size_t bitsPerPixel = ::DirectX::BitsPerPixel(textureDescription.Format);

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
		textureDescription.Format = GetDXGIFormat(lightMap.Format);
		textureDescription.SampleDesc.Count = 1;
		textureDescription.SampleDesc.Quality = 0;
		textureDescription.Usage = D3D11_USAGE_IMMUTABLE;
		textureDescription.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		textureDescription.CPUAccessFlags = 0;
		textureDescription.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		const size_t bitsPerPixel = ::DirectX::BitsPerPixel(textureDescription.Format);

		std::array<D3D11_SUBRESOURCE_DATA, CubeFaceCount * MaxMipMaps> initialResourceData = {};

		for (u32 faceIndex = 0; faceIndex < CubeFaceCount; faceIndex++)
		{
			for (u32 mipIndex = 0; mipIndex < textureDescription.MipLevels; mipIndex++)
			{
				const ivec2 mipMapSize = (lightMap.Size >> static_cast<i32>(mipIndex));
				initialResourceData[LightMapCubeFaceIndices[faceIndex] * textureDescription.MipLevels + mipIndex] = 
				{ 
					lightMap.DataPointers[faceIndex][mipIndex], 
					static_cast<UINT>(GetMemoryPitch(mipMapSize, bitsPerPixel, false)), 
					0 
				};
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
