#include "D3D11Texture.h"
#include "D3D11GraphicsTypeHelpers.h"
#include "Graphics/Utilities/TextureCompression.h"
#include <DirectXTex.h>

namespace Comfy::Render
{
	using namespace Graphics;

	namespace
	{
		// NOTE: This doesn't seem to be defined anywhere but this should cover even the largest supported textures
		constexpr size_t MaxMipMaps = 16;
		constexpr size_t CubeFaceCount = 6;

		constexpr u32 BlockCompressionAlignment = 4;
		constexpr u32 UnboundTextureSlot = 0xFFFFFFFF;
		constexpr u32 UnboundSamplerSlot = 0xFFFFFFFF;

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

		constexpr DXGI_FORMAT DXGITypedDepthFormatToTypeless(DXGI_FORMAT typedDepthFormat)
		{
			if (typedDepthFormat == DXGI_FORMAT_D32_FLOAT)
				return DXGI_FORMAT_R32_TYPELESS;

			if (typedDepthFormat == DXGI_FORMAT_D24_UNORM_S8_UINT)
				return DXGI_FORMAT_R24G8_TYPELESS;

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}

		constexpr DXGI_FORMAT DXGITypelessDepthFormatToTyped(DXGI_FORMAT typelessDepthFormat)
		{
			if (typelessDepthFormat == DXGI_FORMAT_R32_TYPELESS)
				return DXGI_FORMAT_D32_FLOAT;

			if (typelessDepthFormat == DXGI_FORMAT_R24G8_TYPELESS)
				return DXGI_FORMAT_D24_UNORM_S8_UINT;

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}

		constexpr DXGI_FORMAT DXGITypelessDepthFormatToTypedColorFormat(DXGI_FORMAT typelessDepthFormat)
		{
			if (typelessDepthFormat == DXGI_FORMAT_R32_TYPELESS)
				return DXGI_FORMAT_R32_FLOAT;

			if (typelessDepthFormat == DXGI_FORMAT_R24G8_TYPELESS)
				return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}

		constexpr D3D11_SHADER_RESOURCE_VIEW_DESC CreateDepthTextureResourceViewDesc(const D3D11_TEXTURE2D_DESC& typelessDepthTextureDesc)
		{
			D3D11_SHADER_RESOURCE_VIEW_DESC outViewDesc = {};
			outViewDesc.Format = DXGITypelessDepthFormatToTypedColorFormat(typelessDepthTextureDesc.Format);
			outViewDesc.ViewDimension = (typelessDepthTextureDesc.SampleDesc.Count == 1) ? D3D11_SRV_DIMENSION_TEXTURE2D : D3D11_SRV_DIMENSION_TEXTURE2DMS;
			outViewDesc.Texture2D.MostDetailedMip = 0;
			outViewDesc.Texture2D.MipLevels = 1;
			return outViewDesc;
		}

		constexpr D3D11_DEPTH_STENCIL_VIEW_DESC CreateDepthStencilResourceViewDesc(const D3D11_TEXTURE2D_DESC& typelessDepthTextureDesc)
		{
			D3D11_DEPTH_STENCIL_VIEW_DESC outViewDesc = {};
			outViewDesc.Format = DXGITypelessDepthFormatToTyped(typelessDepthTextureDesc.Format);
			outViewDesc.ViewDimension = (typelessDepthTextureDesc.SampleDesc.Count == 1) ? D3D11_DSV_DIMENSION_TEXTURE2D : D3D11_DSV_DIMENSION_TEXTURE2DMS;
			outViewDesc.Texture2D.MipSlice = 0;
			return outViewDesc;
		}

		std::unique_ptr<u8[]> StageAndCopyTexture2DToCPU(D3D11& d3d11, ID3D11Texture2D& sourceTexture2D, D3D11_TEXTURE2D_DESC textureDesc)
		{
			assert(textureDesc.Format == DXGI_FORMAT_R8G8B8A8_UNORM);
			const size_t formatBitsPerPixel = ::DirectX::BitsPerPixel(textureDesc.Format);

			textureDesc.Usage = D3D11_USAGE_STAGING;
			textureDesc.BindFlags = 0;
			textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
			textureDesc.MiscFlags = 0;

			ComPtr<ID3D11Texture2D> stagingTexture;
			if (FAILED(d3d11.Device->CreateTexture2D(&textureDesc, nullptr, &stagingTexture)) || stagingTexture == nullptr)
				return nullptr;

			d3d11.ImmediateContext->CopyResource(stagingTexture.Get(), &sourceTexture2D);

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			if (FAILED(d3d11.ImmediateContext->Map(stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource)))
				return nullptr;

			const size_t resourceStride = mappedResource.RowPitch;
			const size_t resourceSize = mappedResource.DepthPitch;
			const u8* resourceData = static_cast<const u8*>(mappedResource.pData);

			const size_t outputStride = (textureDesc.Width * formatBitsPerPixel) / CHAR_BIT;
			const size_t outputSize = textureDesc.Height * outputStride;
			auto cpuPixels = std::make_unique<u8[]>(outputSize);

			if (resourceSize == outputSize)
			{
				std::memcpy(cpuPixels.get(), resourceData, resourceSize);
			}
			else // NOTE: Tightly pack together all pixels as that's the format they are expected to be in everywhere else in the code
			{
				assert(resourceSize == (resourceStride * textureDesc.Height));

				for (size_t y = 0; y < textureDesc.Height; y++)
					std::memcpy(&cpuPixels[outputStride * y], &resourceData[resourceStride * y], outputStride);
			}

			d3d11.ImmediateContext->Unmap(stagingTexture.Get(), 0);
			return cpuPixels;
		}
	}

	D3D11Texture1DAndView::D3D11Texture1DAndView(D3D11& d3d11, i32 width, const void* pixelData, DXGI_FORMAT format, D3D11_USAGE usage)
	{
		TextureDesc.Width = width;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.Format = format;
		TextureDesc.Usage = usage;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		TextureDesc.MiscFlags = 0;

		d3d11.Device->CreateTexture1D(&TextureDesc, (pixelData == nullptr) ? nullptr : PtrArg<D3D11_SUBRESOURCE_DATA>({ pixelData, 0, 0 }), &Texture);
		d3d11.Device->CreateShaderResourceView(Texture.Get(), nullptr, &TextureView);
	}

	void D3D11Texture1DAndView::UploadDataIfDynamic(D3D11& d3d11, size_t dataSize, const void* pixelData)
	{
		assert(TextureDesc.Usage == D3D11_USAGE_DYNAMIC);

		D3D11Helper::MemoryMapAndCopy(d3d11.ImmediateContext.Get(), Texture.Get(), pixelData, dataSize);
	}

	D3D11Texture2DAndView::D3D11Texture2DAndView(D3D11& d3d11, const Tex& tex, D3D11_USAGE usage) : D3D11RefForDeferedDeletion(d3d11)
	{
		LastBoundSlot = UnboundTextureSlot;

		if (tex.GetSignature() == TxpSig::Texture2D)
		{
			assert(tex.MipMapsArray.size() == 1);
			assert(!tex.MipMapsArray.empty() && !tex.MipMapsArray.front().empty());

			const auto& mipMaps = tex.MipMapsArray.front();
			const auto& baseMipMap = mipMaps.front();

			TextureFormat = baseMipMap.Format;
			TextureDesc.Width = baseMipMap.Size.x;
			TextureDesc.Height = baseMipMap.Size.y;
			TextureDesc.MipLevels = static_cast<UINT>(mipMaps.size());
			TextureDesc.ArraySize = static_cast<UINT>(tex.MipMapsArray.size());
			TextureDesc.Format = GetDXGIFormat(TextureFormat);
			TextureDesc.SampleDesc.Count = 1;
			TextureDesc.SampleDesc.Quality = 0;
			TextureDesc.Usage = usage;
			TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			TextureDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
			TextureDesc.MiscFlags = 0;

			std::array<D3D11_SUBRESOURCE_DATA, MaxMipMaps> initialResourceData;

			// NOTE: Natively unsupported 24-bit format so it needs to be padded first
			if (baseMipMap.Format == TextureFormat::RGB8)
			{
				TextureDesc.Format = GetDXGIFormat(TextureFormat::RGBA8);
				std::array<std::unique_ptr<u8[]>, MaxMipMaps> rgbaBuffers;

				for (size_t i = 0; i < mipMaps.size(); i++)
				{
					auto& resource = initialResourceData[i];
					const auto& mipMap = mipMaps[i];

					const size_t rgbaByteSize = Utilities::TextureFormatByteSize(mipMap.Size, TextureFormat::RGBA8);
					rgbaBuffers[i] = std::make_unique<u8[]>(rgbaByteSize);

					Utilities::ConvertRGBToRGBA(mipMap.Size, mipMap.Data.get(), mipMap.DataSize, rgbaBuffers[i].get(), rgbaByteSize);

					resource.pSysMem = rgbaBuffers[i].get();
					resource.SysMemPitch = static_cast<UINT>(GetMemoryPitch(mipMap.Size, ::DirectX::BitsPerPixel(TextureDesc.Format), false));
					resource.SysMemSlicePitch = 0;
				}

				d3d11.Device->CreateTexture2D(&TextureDesc, initialResourceData.data(), &Texture);
			}
			else
			{
				const bool usesBlockCompression = ::DirectX::IsCompressed(TextureDesc.Format);
				const size_t bitsPerPixel = ::DirectX::BitsPerPixel(TextureDesc.Format);

				if (usesBlockCompression)
					PadTextureDimensions(TextureDesc.Width, TextureDesc.Height, BlockCompressionAlignment);

				for (size_t i = 0; i < mipMaps.size(); i++)
					initialResourceData[i] = CreateMipMapSubresourceData(mipMaps[i], usesBlockCompression, bitsPerPixel);

				d3d11.Device->CreateTexture2D(&TextureDesc, initialResourceData.data(), &Texture);
			}

			d3d11.Device->CreateShaderResourceView(Texture.Get(), nullptr, &TextureView);
		}
		else if (tex.GetSignature() == TxpSig::CubeMap)
		{
			assert(tex.MipMapsArray.size() == 6 && tex.MipMapsArray.front().size() > 0);

			auto& mipMaps = tex.MipMapsArray.front();
			auto& baseMipMap = mipMaps.front();

			TextureFormat = baseMipMap.Format;
			TextureDesc.Width = baseMipMap.Size.x;
			TextureDesc.Height = baseMipMap.Size.y;
			TextureDesc.MipLevels = static_cast<UINT>(mipMaps.size());
			TextureDesc.ArraySize = static_cast<UINT>(tex.MipMapsArray.size());
			TextureDesc.Format = GetDXGIFormat(TextureFormat);
			TextureDesc.SampleDesc.Count = 1;
			TextureDesc.SampleDesc.Quality = 0;
			TextureDesc.Usage = usage;
			TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			TextureDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
			TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

			const bool usesBlockCompression = ::DirectX::IsCompressed(TextureDesc.Format);
			const size_t bitsPerPixel = ::DirectX::BitsPerPixel(TextureDesc.Format);

			if (usesBlockCompression)
				PadTextureDimensions(TextureDesc.Width, TextureDesc.Height, BlockCompressionAlignment);

			std::array<D3D11_SUBRESOURCE_DATA, CubeFaceCount * MaxMipMaps> initialResourceData;
			for (u32 faceIndex = 0; faceIndex < CubeFaceCount; faceIndex++)
			{
				for (u32 mipIndex = 0; mipIndex < TextureDesc.MipLevels; mipIndex++)
					initialResourceData[TexCubeFaceIndices[faceIndex] * TextureDesc.MipLevels + mipIndex] = CreateMipMapSubresourceData(tex.MipMapsArray[faceIndex][mipIndex], usesBlockCompression, bitsPerPixel);
			}

			d3d11.Device->CreateTexture2D(&TextureDesc, initialResourceData.data(), &Texture);
			d3d11.Device->CreateShaderResourceView(Texture.Get(), nullptr, &TextureView);
		}
		else
		{
			assert(false);
		}
	}

	D3D11Texture2DAndView::D3D11Texture2DAndView(D3D11& d3d11, const LightMapIBL& lightMap) : D3D11RefForDeferedDeletion(d3d11)
	{
		LastBoundSlot = UnboundTextureSlot;

		u32 mipMapLevels = 1;
		for (u32 i = 0; i < lightMap.DataPointers[0].size(); i++)
		{
			if (lightMap.DataPointers[0][i] == nullptr)
				break;
			mipMapLevels = i + 1;
		}

		TextureFormat = TextureFormat::Unknown;
		TextureDesc.Width = lightMap.Size.x;
		TextureDesc.Height = lightMap.Size.y;
		TextureDesc.MipLevels = mipMapLevels;
		TextureDesc.ArraySize = CubeFaceCount;
		TextureDesc.Format = GetDXGIFormat(lightMap.Format);
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.Usage = D3D11_USAGE_IMMUTABLE;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDesc.CPUAccessFlags = 0;
		TextureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		const size_t bitsPerPixel = ::DirectX::BitsPerPixel(TextureDesc.Format);

		std::array<D3D11_SUBRESOURCE_DATA, CubeFaceCount * MaxMipMaps> initialResourceData = {};
		for (u32 faceIndex = 0; faceIndex < CubeFaceCount; faceIndex++)
		{
			for (u32 mipIndex = 0; mipIndex < TextureDesc.MipLevels; mipIndex++)
			{
				const ivec2 mipMapSize = (lightMap.Size >> static_cast<i32>(mipIndex));
				initialResourceData[LightMapCubeFaceIndices[faceIndex] * TextureDesc.MipLevels + mipIndex] =
				{
					lightMap.DataPointers[faceIndex][mipIndex],
					static_cast<UINT>(GetMemoryPitch(mipMapSize, bitsPerPixel, false)),
					0
				};
			}
		}

		d3d11.Device->CreateTexture2D(&TextureDesc, initialResourceData.data(), &Texture);
		d3d11.Device->CreateShaderResourceView(Texture.Get(), nullptr, &TextureView);
	}

	D3D11Texture2DAndView::D3D11Texture2DAndView(D3D11& d3d11, ivec2 size, const u32* rgbaBuffer, D3D11_USAGE usage) : D3D11RefForDeferedDeletion(d3d11)
	{
		LastBoundSlot = UnboundTextureSlot;

		TextureFormat = TextureFormat::RGBA8;
		TextureDesc.Width = size.x;
		TextureDesc.Height = size.y;
		TextureDesc.MipLevels = 1;
		TextureDesc.ArraySize = 1;
		TextureDesc.Format = GetDXGIFormat(TextureFormat);
		TextureDesc.SampleDesc.Count = 1;
		TextureDesc.SampleDesc.Quality = 0;
		TextureDesc.Usage = usage;
		TextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		TextureDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		TextureDesc.MiscFlags = 0;

		d3d11.Device->CreateTexture2D(&TextureDesc, PtrArg<D3D11_SUBRESOURCE_DATA>({ rgbaBuffer, static_cast<UINT>(GetMemoryPitch(size, ::DirectX::BitsPerPixel(TextureDesc.Format))), 0 }), &Texture);
		d3d11.Device->CreateShaderResourceView(Texture.Get(), nullptr, &TextureView);
	}

	D3D11Texture2DAndView::D3D11Texture2DAndView(D3D11& d3d11, const D3D11RenderTargetAndView& sourceRenderTargetToCopy) : D3D11RefForDeferedDeletion(d3d11)
	{
		LastBoundSlot = UnboundTextureSlot;
		CreateCopyFrom(d3d11, sourceRenderTargetToCopy);
	}

	D3D11Texture2DAndView::~D3D11Texture2DAndView()
	{
		D3D11RefForDeferedDeletion.DeferObjectDeletion(TextureView);
	}

	void D3D11Texture2DAndView::Bind(D3D11& d3d11, u32 textureSlot) const
	{
		assert(textureSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);

		LastBoundSlot = textureSlot;
		d3d11.ImmediateContext->PSSetShaderResources(textureSlot, 1, PtrArg<ID3D11ShaderResourceView*>(TextureView.Get()));
	}

	void D3D11Texture2DAndView::UnBind(D3D11& d3d11) const
	{
		assert(LastBoundSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);

		d3d11.ImmediateContext->PSSetShaderResources(LastBoundSlot, 1, PtrArg<ID3D11ShaderResourceView*>(nullptr));
		LastBoundSlot = UnboundTextureSlot;
	}

	void D3D11Texture2DAndView::UploadDataIfDynamic(D3D11& d3d11, const Tex& tex)
	{
		// HACK: This all needs some rethinking...
		assert(TextureDesc.Usage == D3D11_USAGE_DYNAMIC);
		assert(tex.GetFormat() == TextureFormat);
		assert(tex.MipMapsArray.size() == 1 && tex.MipMapsArray.front().size() == 1);

		D3D11_MAPPED_SUBRESOURCE mappedTexture = {};
		if (SUCCEEDED(d3d11.ImmediateContext->Map(Texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedTexture)))
		{
			const size_t resourceStride = mappedTexture.RowPitch;
			const size_t resourceSize = mappedTexture.DepthPitch;
			u8* resourceData = static_cast<u8*>(mappedTexture.pData);
			assert(resourceData != nullptr);

			constexpr u32 rgbaBitsPerPixel = (sizeof(u32) * CHAR_BIT);
			auto& baseMip = tex.MipMapsArray[0][0];

			const size_t inputStride = (TextureDesc.Width * rgbaBitsPerPixel) / CHAR_BIT;
			const size_t inputSize = TextureDesc.Height * inputStride;
			const u8* inputData = baseMip.Data.get();
			assert(inputSize == baseMip.DataSize);

			if (resourceSize == inputSize)
			{
				std::memcpy(resourceData, inputData, inputSize);
			}
			else
			{
				assert(resourceSize == (resourceStride * TextureDesc.Height));

				for (size_t y = 0; y < TextureDesc.Height; y++)
					std::memcpy(&resourceData[resourceStride * y], &inputData[inputStride * y], inputStride);
			}

			d3d11.ImmediateContext->Unmap(Texture.Get(), 0);
		}
	}

	void D3D11Texture2DAndView::CreateCopyFrom(D3D11& d3d11, const D3D11RenderTargetAndView& sourceRenderTargetToCopy)
	{
		assert(sourceRenderTargetToCopy.ColorTextureDesc.Format == GetDXGIFormat(TextureFormat::RGBA8));
		TextureFormat = TextureFormat::RGBA8;

		if (Texture == nullptr || sourceRenderTargetToCopy.ColorTextureDesc.Width != TextureDesc.Width || sourceRenderTargetToCopy.ColorTextureDesc.Height != TextureDesc.Height)
		{
			d3d11.Device->CreateTexture2D(&sourceRenderTargetToCopy.ColorTextureDesc, nullptr, &Texture);
			d3d11.Device->CreateShaderResourceView(Texture.Get(), nullptr, &TextureView);
		}

		TextureDesc = sourceRenderTargetToCopy.ColorTextureDesc;
		d3d11.ImmediateContext->CopyResource(Texture.Get(), sourceRenderTargetToCopy.ColorTexture.Get());
	}

	ivec2 D3D11Texture2DAndView::GetSize() const
	{
		return { static_cast<i32>(TextureDesc.Width), static_cast<i32>(TextureDesc.Height) };
	}

	bool D3D11Texture2DAndView::GetIsDynamic() const
	{
		return (TextureDesc.Usage == D3D11_USAGE_DYNAMIC);
	}

	bool D3D11Texture2DAndView::GetIsCubeMap() const
	{
		return (TextureDesc.MiscFlags == D3D11_RESOURCE_MISC_TEXTURECUBE);
	}

	D3D11TextureSampler::D3D11TextureSampler(D3D11& d3d11, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV)
		: D3D11TextureSampler(d3d11, filter, addressModeUV, addressModeUV, 0.0f, D3D11_MIN_MAXANISOTROPY)
	{
	}

	D3D11TextureSampler::D3D11TextureSampler(D3D11& d3d11, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV)
		: D3D11TextureSampler(d3d11, filter, addressModeU, addressModeV, 0.0f, D3D11_MIN_MAXANISOTROPY)
	{
	}

	D3D11TextureSampler::D3D11TextureSampler(D3D11& d3d11, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias, int anisotropicFiltering)
	{
		constexpr vec4 transparentBorderColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		LastBoundSlot = UnboundSamplerSlot;

		SamplerDesc.Filter = filter;
		SamplerDesc.AddressU = addressModeU;
		SamplerDesc.AddressV = addressModeV;
		SamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		SamplerDesc.MipLODBias = std::clamp(mipMapBias, D3D11_MIP_LOD_BIAS_MIN, D3D11_MIP_LOD_BIAS_MAX);
		SamplerDesc.MaxAnisotropy = anisotropicFiltering;
		SamplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		SamplerDesc.BorderColor[0] = transparentBorderColor[0];
		SamplerDesc.BorderColor[1] = transparentBorderColor[1];
		SamplerDesc.BorderColor[2] = transparentBorderColor[2];
		SamplerDesc.BorderColor[3] = transparentBorderColor[3];
		SamplerDesc.MinLOD = 0.0f;
		SamplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		d3d11.Device->CreateSamplerState(&SamplerDesc, &SamplerState);
	}

	void D3D11TextureSampler::Bind(D3D11& d3d11, u32 samplerSlot) const
	{
		assert(samplerSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

		LastBoundSlot = samplerSlot;
		d3d11.ImmediateContext->PSSetSamplers(samplerSlot, 1, PtrArg<ID3D11SamplerState*>(SamplerState.Get()));
	}

	void D3D11TextureSampler::UnBind(D3D11& d3d11) const
	{
		assert(LastBoundSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

		d3d11.ImmediateContext->PSSetSamplers(LastBoundSlot, 1, PtrArg<ID3D11SamplerState*>(nullptr));
		LastBoundSlot = UnboundSamplerSlot;
	}

	D3D11RenderTargetAndView::D3D11RenderTargetAndView(D3D11& d3d11, ivec2 size, DXGI_FORMAT colorFormat, u32 multiSampleCount)
		: D3D11RenderTargetAndView(d3d11, size, colorFormat, DXGI_FORMAT_UNKNOWN, multiSampleCount)
	{
	}

	D3D11RenderTargetAndView::D3D11RenderTargetAndView(D3D11& d3d11, ivec2 size, DXGI_FORMAT colorFormat, DXGI_FORMAT depthBufferFormat, u32 multiSampleCount)
	{
		LastBoundSlot = UnboundTextureSlot;

		if (colorFormat != DXGI_FORMAT_UNKNOWN)
		{
			ColorTextureDesc.Width = size.x;
			ColorTextureDesc.Height = size.y;
			ColorTextureDesc.MipLevels = 1;
			ColorTextureDesc.ArraySize = 1;
			ColorTextureDesc.Format = colorFormat;
			ColorTextureDesc.SampleDesc.Count = multiSampleCount;
			ColorTextureDesc.SampleDesc.Quality = 0;
			ColorTextureDesc.Usage = D3D11_USAGE_DEFAULT;
			// NOTE: For now always enable D3D11_BIND_SHADER_RESOURCE and create a ColorTextureView. Could be made optional in the future if ever needed
			ColorTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			ColorTextureDesc.CPUAccessFlags = 0;
			ColorTextureDesc.MiscFlags = 0;

			d3d11.Device->CreateTexture2D(&ColorTextureDesc, nullptr, &ColorTexture);
			if (ColorTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(ColorTexture.Get(), nullptr, &ColorTextureView);

			d3d11.Device->CreateRenderTargetView(ColorTexture.Get(), nullptr, &RenderTargetView);
		}

		if (depthBufferFormat != DXGI_FORMAT_UNKNOWN)
		{
			assert(depthBufferFormat == DXGI_FORMAT_D32_FLOAT || depthBufferFormat == DXGI_FORMAT_D24_UNORM_S8_UINT);

			DepthTextureDesc.Width = size.x;
			DepthTextureDesc.Height = size.y;
			DepthTextureDesc.MipLevels = 1;
			DepthTextureDesc.ArraySize = 1;
			DepthTextureDesc.Format = DXGITypedDepthFormatToTypeless(depthBufferFormat);
			DepthTextureDesc.SampleDesc.Count = multiSampleCount;
			DepthTextureDesc.SampleDesc.Quality = 0;
			DepthTextureDesc.Usage = D3D11_USAGE_DEFAULT;
			// NOTE: For now always enable D3D11_BIND_SHADER_RESOURCE and create a DepthTextureView. Could be made optional in the future if ever needed
			DepthTextureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
			DepthTextureDesc.CPUAccessFlags = 0;
			DepthTextureDesc.MiscFlags = 0;

			d3d11.Device->CreateTexture2D(&DepthTextureDesc, nullptr, &DepthTexture);
			if (DepthTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(DepthTexture.Get(), PtrArg(CreateDepthTextureResourceViewDesc(DepthTextureDesc)), &DepthTextureView);

			d3d11.Device->CreateDepthStencilView(DepthTexture.Get(), PtrArg(CreateDepthStencilResourceViewDesc(DepthTextureDesc)), &DepthStencilView);
		}
	}

	void D3D11RenderTargetAndView::Bind(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->OMSetRenderTargets(1, PtrArg<ID3D11RenderTargetView*>(RenderTargetView.Get()), DepthStencilView.Get());
	}

	void D3D11RenderTargetAndView::BindAndSetViewport(D3D11& d3d11) const
	{
		d3d11.SetViewport(vec2(GetSize()));
		d3d11.ImmediateContext->OMSetRenderTargets(1, PtrArg<ID3D11RenderTargetView*>(RenderTargetView.Get()), DepthStencilView.Get());
	}

	void D3D11RenderTargetAndView::UnBind(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->OMSetRenderTargets(1, PtrArg<ID3D11RenderTargetView*>(nullptr), nullptr);
	}

	void D3D11RenderTargetAndView::BindColorTexturePS(D3D11& d3d11, u32 textureSlot) const
	{
		assert(textureSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		assert(GetHasColorBuffer());

		LastBoundSlot = textureSlot;
		d3d11.ImmediateContext->PSSetShaderResources(textureSlot, 1, PtrArg<ID3D11ShaderResourceView*>({ ColorTextureView.Get() }));
	}

	void D3D11RenderTargetAndView::BindDepthTexturePS(D3D11& d3d11, u32 textureSlot) const
	{
		assert(textureSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);
		assert(GetHasDepthBuffer());

		LastBoundSlot = textureSlot;
		d3d11.ImmediateContext->PSSetShaderResources(textureSlot, 1, PtrArg<ID3D11ShaderResourceView*>({ DepthTextureView.Get() }));
	}

	void D3D11RenderTargetAndView::UnBindPS(D3D11& d3d11, u32 textureSlot) const
	{
		assert(LastBoundSlot < D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT);

		d3d11.ImmediateContext->PSSetShaderResources(LastBoundSlot, 1, PtrArg<ID3D11ShaderResourceView*>({ nullptr }));
		LastBoundSlot = UnboundTextureSlot;
	}

	void D3D11RenderTargetAndView::ClearColor(D3D11& d3d11, vec4 color)
	{
		if (GetHasColorBuffer())
		{
			d3d11.ImmediateContext->ClearRenderTargetView(RenderTargetView.Get(), glm::value_ptr(color));
		}
	}

	void D3D11RenderTargetAndView::ClearDepth(D3D11& d3d11, f32 depth)
	{
		if (GetHasDepthBuffer())
		{
			if (DepthTextureDesc.Format == DXGI_FORMAT_R32_TYPELESS)
				d3d11.ImmediateContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH, depth, 0x00);
			else
				d3d11.ImmediateContext->ClearDepthStencilView(DepthStencilView.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, depth, 0xFF);
		}
	}

	void D3D11RenderTargetAndView::ClearColorAndDepth(D3D11& d3d11, vec4 color, f32 depth)
	{
		ClearColor(d3d11, color);
		ClearDepth(d3d11, depth);
	}

	ivec2 D3D11RenderTargetAndView::GetSize() const
	{
		if (GetHasColorBuffer())
			return { static_cast<i32>(ColorTextureDesc.Width), static_cast<i32>(ColorTextureDesc.Height) };
		else if (GetHasDepthBuffer())
			return { static_cast<i32>(DepthTextureDesc.Width), static_cast<i32>(DepthTextureDesc.Height) };
		else
			return { 0, 0 };
	}

	void D3D11RenderTargetAndView::RecreateWithNewSizeIfDifferent(D3D11& d3d11, ivec2 newSize)
	{
		newSize.x = std::clamp<i32>(newSize.x, D3D11MinTexture2DSize.x, D3D11MaxTexture2DSize.x);
		newSize.y = std::clamp<i32>(newSize.y, D3D11MinTexture2DSize.y, D3D11MaxTexture2DSize.y);
		if (newSize == GetSize())
			return;

		if (GetHasColorBuffer())
		{
			ColorTextureDesc.Width = newSize.x;
			ColorTextureDesc.Height = newSize.y;

			d3d11.Device->CreateTexture2D(&ColorTextureDesc, nullptr, &ColorTexture);
			if (ColorTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(ColorTexture.Get(), nullptr, &ColorTextureView);

			d3d11.Device->CreateRenderTargetView(ColorTexture.Get(), nullptr, &RenderTargetView);
		}

		if (GetHasDepthBuffer())
		{
			DepthTextureDesc.Width = newSize.x;
			DepthTextureDesc.Height = newSize.y;

			d3d11.Device->CreateTexture2D(&DepthTextureDesc, nullptr, &DepthTexture);
			if (DepthTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(DepthTexture.Get(), PtrArg(CreateDepthTextureResourceViewDesc(DepthTextureDesc)), &DepthTextureView);

			d3d11.Device->CreateDepthStencilView(DepthTexture.Get(), PtrArg(CreateDepthStencilResourceViewDesc(DepthTextureDesc)), &DepthStencilView);
		}
	}

	void D3D11RenderTargetAndView::RecreateWithNewColorFormatIfDifferent(D3D11& d3d11, DXGI_FORMAT newColorFormat)
	{
		if (ColorTextureDesc.Format == newColorFormat)
			return;

		if (GetHasColorBuffer())
		{
			ColorTextureDesc.Format = newColorFormat;

			d3d11.Device->CreateTexture2D(&ColorTextureDesc, nullptr, &ColorTexture);
			if (ColorTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(ColorTexture.Get(), nullptr, &ColorTextureView);

			d3d11.Device->CreateRenderTargetView(ColorTexture.Get(), nullptr, &RenderTargetView);
		}
	}

	u32 D3D11RenderTargetAndView::GetMultiSampleCount() const
	{
		if (GetHasColorBuffer())
			return ColorTextureDesc.SampleDesc.Count;
		else if (GetHasDepthBuffer())
			return DepthTextureDesc.SampleDesc.Count;
		else
			return 0;
	}

	void D3D11RenderTargetAndView::RecreateWithNewMultiSampleCountIfDifferent(D3D11& d3d11, u32 newMultiSampleCount)
	{
		newMultiSampleCount = std::clamp<u32>(newMultiSampleCount, 1, D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT);
		if (newMultiSampleCount == GetMultiSampleCount())
			return;

		bool isEitherUnsupported = false;

		if (GetHasColorBuffer() && !isEitherUnsupported)
		{
			ColorTextureDesc.SampleDesc.Count = newMultiSampleCount;
			if (FAILED(d3d11.Device->CreateTexture2D(&ColorTextureDesc, nullptr, &ColorTexture)))
				isEitherUnsupported = true;
		}

		if (GetHasDepthBuffer() && !isEitherUnsupported)
		{
			DepthTextureDesc.SampleDesc.Count = newMultiSampleCount;
			if (FAILED(d3d11.Device->CreateTexture2D(&DepthTextureDesc, nullptr, &DepthTexture)))
				isEitherUnsupported = true;
		}

		// NOTE: Resort to no MSAA if unsupported by the driver
		if (isEitherUnsupported)
		{
			if (GetHasColorBuffer())
			{
				ColorTextureDesc.SampleDesc.Count = 1;
				d3d11.Device->CreateTexture2D(&ColorTextureDesc, nullptr, &ColorTexture);
			}
			if (GetHasDepthBuffer())
			{
				DepthTextureDesc.SampleDesc.Count = 1;
				d3d11.Device->CreateTexture2D(&DepthTextureDesc, nullptr, &DepthTexture);
			}
		}

		if (GetHasColorBuffer())
		{
			if (ColorTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(ColorTexture.Get(), nullptr, &ColorTextureView);

			d3d11.Device->CreateRenderTargetView(ColorTexture.Get(), nullptr, &RenderTargetView);
		}

		if (GetHasDepthBuffer())
		{
			if (DepthTextureDesc.BindFlags & D3D11_BIND_SHADER_RESOURCE)
				d3d11.Device->CreateShaderResourceView(DepthTexture.Get(), PtrArg(CreateDepthTextureResourceViewDesc(DepthTextureDesc)), &DepthTextureView);

			d3d11.Device->CreateDepthStencilView(DepthTexture.Get(), PtrArg(CreateDepthStencilResourceViewDesc(DepthTextureDesc)), &DepthStencilView);
		}
	}

	bool D3D11RenderTargetAndView::GetHasColorBuffer() const
	{
		return (ColorTextureDesc.Format != DXGI_FORMAT_UNKNOWN);
	}

	bool D3D11RenderTargetAndView::GetHasDepthBuffer() const
	{
		return (DepthTextureDesc.Format != DXGI_FORMAT_UNKNOWN);
	}

	std::unique_ptr<u8[]> D3D11RenderTargetAndView::CopyColorPixelsBackToCPU(D3D11& d3d11)
	{
		return (ColorTexture != nullptr) ? StageAndCopyTexture2DToCPU(d3d11, *ColorTexture.Get(), ColorTextureDesc) : nullptr;
	}
}
