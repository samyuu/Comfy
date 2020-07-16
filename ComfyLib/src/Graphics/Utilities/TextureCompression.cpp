#include "TextureCompression.h"
#include "IO/Path.h"
#include "Misc/UTF8.h"
#include "Core/Win32/ComfyWindows.h"
#include <DirectXTex.h>

#pragma comment(lib, "DirectXTex.lib")

namespace Comfy::Graphics::Utilities
{
	namespace
	{
		constexpr DXGI_FORMAT TextureFormatToDXGI(TextureFormat format)
		{
			switch (format)
			{
			case TextureFormat::A8:
				return DXGI_FORMAT_R8_UINT;

			case TextureFormat::RGB8:
				return DXGI_FORMAT_UNKNOWN;

			case TextureFormat::RGBA8:
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			case TextureFormat::RGB5:
				return DXGI_FORMAT_UNKNOWN;

			case TextureFormat::RGB5_A1:
				return DXGI_FORMAT_UNKNOWN;

			case TextureFormat::RGBA4:
				return DXGI_FORMAT_UNKNOWN;

			case TextureFormat::DXT1:
				return DXGI_FORMAT_BC1_UNORM;

			case TextureFormat::DXT1a:
				return DXGI_FORMAT_BC1_UNORM_SRGB;

			case TextureFormat::DXT3:
				return DXGI_FORMAT_BC2_UNORM;

			case TextureFormat::DXT5:
				return DXGI_FORMAT_BC3_UNORM;

			case TextureFormat::RGTC1:
				return DXGI_FORMAT_BC4_UNORM;

			case TextureFormat::RGTC2:
				return DXGI_FORMAT_BC5_UNORM;

			case TextureFormat::L8:
				return DXGI_FORMAT_A8_UNORM;

			case TextureFormat::L8A8:
				return DXGI_FORMAT_A8P8;

			default:
				return DXGI_FORMAT_UNKNOWN;
			}
		}

		constexpr TextureFormat DXGIFormatToTextureFormat(DXGI_FORMAT format)
		{
			switch (format)
			{
			case DXGI_FORMAT_R8_UINT:
				return TextureFormat::A8;

			case DXGI_FORMAT_R8G8B8A8_UNORM:
				return TextureFormat::RGBA8;

			case DXGI_FORMAT_BC1_UNORM:
				return TextureFormat::DXT1;

			case DXGI_FORMAT_BC1_UNORM_SRGB:
				return TextureFormat::DXT1a;

			case DXGI_FORMAT_BC2_UNORM:
				return TextureFormat::DXT3;

			case DXGI_FORMAT_BC3_UNORM:
				return TextureFormat::DXT5;

			case DXGI_FORMAT_BC4_UNORM:
				return TextureFormat::RGTC1;

			case DXGI_FORMAT_BC5_UNORM:
				return TextureFormat::RGTC2;

			case DXGI_FORMAT_A8_UNORM:
				return TextureFormat::L8;

			case DXGI_FORMAT_A8P8:
				return TextureFormat::L8A8;

			default:
				return TextureFormat::Unknown;
			}
		}
	}

	size_t TextureFormatBlockSize(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::DXT1:
		case TextureFormat::DXT1a:
		case TextureFormat::RGTC1:
			return 8;

		case TextureFormat::DXT3:
		case TextureFormat::DXT5:
		case TextureFormat::RGTC2:
			return 16;

		default:
			return 0;
		}
	}

	size_t TextureFormatChannelCount(TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::A8:
		case TextureFormat::L8:
		case TextureFormat::RGTC1:
			return 1;

		case TextureFormat::L8A8:
		case TextureFormat::RGTC2:
			return 2;

		case TextureFormat::RGB8:
		case TextureFormat::RGB5:
		case TextureFormat::DXT1:
			return 3;

		case TextureFormat::RGBA8:
		case TextureFormat::RGB5_A1:
		case TextureFormat::RGBA4:
		case TextureFormat::DXT1a:
		case TextureFormat::DXT3:
		case TextureFormat::DXT5:
			return 4;

		default:
			assert(false);
			return 0;
		}
	}

	size_t TextureFormatByteSize(ivec2 size, TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::A8:
		case TextureFormat::L8:
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
		case TextureFormat::L8A8:
			return size.x * size.y * TextureFormatChannelCount(format);

			// BUG: Not entirely accurate but not like these are used anyway
		case TextureFormat::RGB5:
			assert(false);
			return (size.x * size.y * 15) / CHAR_BIT;
		case TextureFormat::RGB5_A1:
			assert(false);
			return (size.x * size.y * 16) / CHAR_BIT;

		case TextureFormat::RGBA4:
			return (size.x * size.y * TextureFormatChannelCount(format)) / 2;

		case TextureFormat::DXT1:
		case TextureFormat::DXT1a:
		case TextureFormat::DXT3:
		case TextureFormat::DXT5:
		case TextureFormat::RGTC1:
		case TextureFormat::RGTC2:
			return glm::max(1, (size.x + 3) / 4) * glm::max(1, (size.y + 3) / 4) * TextureFormatBlockSize(format);

		default:
			assert(false);
			return 0;
		}
	}

	bool DecompressTextureData(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outData, TextureFormat outFormat, size_t outByteSize)
	{
		if (size.x <= 0 || size.y <= 0)
			return false;

		if (inData == nullptr || outData == nullptr)
			return false;

		const auto inFormatDXGI = TextureFormatToDXGI(inFormat);
		const auto outFormatDXGI = TextureFormatToDXGI(outFormat);

		if (inFormatDXGI == DXGI_FORMAT_UNKNOWN || outFormatDXGI == DXGI_FORMAT_UNKNOWN)
			return false;

		const auto expectedInputByteSize = (size.x * size.y * ::DirectX::BitsPerPixel(inFormatDXGI)) / CHAR_BIT;
		if (inByteSize < expectedInputByteSize)
			return false;

		if (inFormat == outFormat)
		{
			std::memcpy(outData, inData, expectedInputByteSize);
			return true;
		}

		auto inputImage = ::DirectX::Image {};
		inputImage.width = size.x;
		inputImage.height = size.y;
		inputImage.format = inFormatDXGI;
		inputImage.rowPitch = TextureFormatByteSize(ivec2(size.x, 1), inFormat);
		inputImage.slicePitch = expectedInputByteSize;
		inputImage.pixels = const_cast<u8*>(inData);

		auto outputImage = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Decompress(inputImage, outFormatDXGI, outputImage)))
			return false;

		if (outByteSize < outputImage.GetPixelsSize())
			return false;

		std::memcpy(outData, outputImage.GetPixels(), outputImage.GetPixelsSize());
		return true;
	}

	bool CompressTextureData(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outData, TextureFormat outFormat, size_t outByteSize)
	{
		if (size.x <= 0 || size.y <= 0)
			return false;

		if (inData == nullptr || outData == nullptr)
			return false;

		const auto inFormatDXGI = TextureFormatToDXGI(inFormat);
		const auto outFormatDXGI = TextureFormatToDXGI(outFormat);

		if (inFormatDXGI == DXGI_FORMAT_UNKNOWN || outFormatDXGI == DXGI_FORMAT_UNKNOWN)
			return false;

		const auto expectedInputByteSize = (size.x * size.y * ::DirectX::BitsPerPixel(inFormatDXGI)) / CHAR_BIT;
		if (inByteSize < expectedInputByteSize)
			return false;

		auto inputImage = ::DirectX::Image {};
		inputImage.width = size.x;
		inputImage.height = size.y;
		inputImage.format = inFormatDXGI;
		inputImage.rowPitch = TextureFormatByteSize(ivec2(size.x, 1), inFormat);
		inputImage.slicePitch = expectedInputByteSize;
		inputImage.pixels = const_cast<u8*>(inData);

		auto compressionFlags = ::DirectX::TEX_COMPRESS_DEFAULT;

		if (true) // NOTE: It seems this yields the best results in most cases
			compressionFlags |= ::DirectX::TEX_COMPRESS_DITHER;

		auto outputImage = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Compress(inputImage, outFormatDXGI, compressionFlags, ::DirectX::TEX_THRESHOLD_DEFAULT, outputImage)))
			return false;

		if (outByteSize < outputImage.GetPixelsSize())
			return false;

		std::memcpy(outData, outputImage.GetPixels(), outputImage.GetPixelsSize());
		return true;
	}

	namespace
	{
		constexpr float CbCrOffset = 0.503929f;
		constexpr float CbCrFactor = 1.003922f;

		constexpr mat3 YCbCrToRGBTransform =
		{
			vec3(+1.5748f, +1.0f, +0.0000f),
			vec3(-0.4681f, +1.0f, -0.1873f),
			vec3(+0.0000f, +1.0f, +1.8556f),
		};

		constexpr mat3 RGBToYCbCrTransform =
		{
			vec3(+0.500004232f, -0.454162151f, -0.0458420813f),
			vec3(+0.212593317f, +0.715214610f, +0.0721921176f),
			vec3(-0.114568502f, -0.385435730f, +0.5000042320f),
		};

		constexpr float PixelU8ToF32(u8 pixel)
		{
			constexpr auto factor = 1.0f / static_cast<float>(std::numeric_limits<u8>::max());
			return static_cast<float>(pixel) * factor;
		}

		constexpr u8 PixelF32ToU8(float pixel)
		{
			constexpr auto factor = static_cast<float>(std::numeric_limits<u8>::max());
			return static_cast<u8>(std::clamp(pixel, 0.0f, 1.0f) * factor);
		}

		constexpr u32 PackU8RGBA(u8 r, u8 g, u8 b, u8 a)
		{
			return
				(static_cast<u32>(a) << 24) |
				(static_cast<u32>(b) << 16) |
				(static_cast<u32>(g) << 8) |
				(static_cast<u32>(r) << 0);
		}

		inline u32 ConvertSinglePixelYACbCrToRGBA(const u8 inYA[2], const u8 inCbCr[2])
		{
			const auto yCbCr = vec3(
				(PixelU8ToF32(inCbCr[1]) * CbCrFactor) - CbCrOffset,
				(PixelU8ToF32(inYA[0])),
				(PixelU8ToF32(inCbCr[0]) * CbCrFactor) - CbCrOffset);

			return PackU8RGBA(
				PixelF32ToU8(glm::dot(yCbCr, YCbCrToRGBTransform[0])),
				PixelF32ToU8(glm::dot(yCbCr, YCbCrToRGBTransform[1])),
				PixelF32ToU8(glm::dot(yCbCr, YCbCrToRGBTransform[2])),
				inYA[1]);
		}

		inline void ConvertSinglePixelRGBAToYACbCr(const u32 inRGBA, u8 outYA[2], u8 outCbCr[2])
		{
			constexpr float cbCrFactor = 1.0f / CbCrFactor;

			const auto rgb = vec3(
				PixelU8ToF32((inRGBA & 0x0000FF)),
				PixelU8ToF32((inRGBA & 0x00FF00) >> 8),
				PixelU8ToF32((inRGBA & 0xFF0000) >> 16));

			outCbCr[0] = PixelF32ToU8((glm::dot(rgb, RGBToYCbCrTransform[2]) + CbCrOffset) * cbCrFactor);
			outCbCr[1] = PixelF32ToU8((glm::dot(rgb, RGBToYCbCrTransform[0]) + CbCrOffset) * cbCrFactor);

			outYA[0] = PixelF32ToU8(glm::dot(rgb, RGBToYCbCrTransform[1]));
			outYA[1] = static_cast<u8>(inRGBA >> 24);
		}
	}

	bool ConvertYACbCrToRGBABuffer(const TexMipMap& mipMapYA, const TexMipMap& mipMapCbCr, u8* outData, size_t outByteSize)
	{
		if (mipMapYA.Format != TextureFormat::RGTC2 || mipMapCbCr.Format != TextureFormat::RGTC2 || (mipMapYA.Size / 2) != mipMapCbCr.Size)
			return false;

		if (outByteSize < TextureFormatByteSize(mipMapYA.Size, TextureFormat::RGBA8))
			return false;

		auto imageFromMip = [](const auto& mip)
		{
			auto image = ::DirectX::Image {};
			image.width = mip.Size.x;
			image.height = mip.Size.y;
			image.format = DXGI_FORMAT_BC5_UNORM;
			image.rowPitch = TextureFormatByteSize(ivec2(mip.Size.x, 1), TextureFormat::RGTC2);
			image.slicePitch = mip.DataSize;
			image.pixels = const_cast<u8*>(mip.Data.get());
			return image;
		};

		auto outputImageYA = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Decompress(imageFromMip(mipMapYA), DXGI_FORMAT_R8G8_UNORM, outputImageYA)))
			return false;

		auto outputImageCbCr = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Decompress(imageFromMip(mipMapCbCr), DXGI_FORMAT_R8G8_UNORM, outputImageCbCr)))
			return false;

		auto outputImageCbCrResized = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Resize(*outputImageCbCr.GetImage(0, 0, 0), mipMapYA.Size.x, mipMapYA.Size.y, ::DirectX::TEX_FILTER_LINEAR | ::DirectX::TEX_FILTER_FORCE_NON_WIC, outputImageCbCrResized)))
			return false;

		const u8* pixelBufferYA = (outputImageYA.GetPixels());
		const u8* pixelBufferCbCrResized = (outputImageCbCrResized.GetPixels());
		u32* outRGBA = reinterpret_cast<u32*>(outData);

		for (size_t y = 0; y < mipMapYA.Size.y; y++)
		{
			for (size_t x = 0; x < mipMapYA.Size.x; x++)
			{
				const auto pixelIndex = (mipMapYA.Size.x * y + x);

				const u8* inYA = &pixelBufferYA[pixelIndex * 2];
				const u8* inCbCr = &pixelBufferCbCrResized[pixelIndex * 2];

				outRGBA[pixelIndex] = ConvertSinglePixelYACbCrToRGBA(inYA, inCbCr);
			}
		}

		return true;
	}

	bool ConvertRGBAToYACbCrBuffer(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outYAData, u8* outCbCrData)
	{
		if (inFormat != TextureFormat::RGBA8)
			return false;

		const auto inRGBAData = reinterpret_cast<const u32*>(inData);

		for (size_t y = 0; y < size.y; y++)
		{
			for (size_t x = 0; x < size.x; x++)
			{
				const auto pixelIndex = (size.x * y + x);

				u8* outYA = &outYAData[pixelIndex * 2];
				u8* outCbCr = &outCbCrData[pixelIndex * 2];

				ConvertSinglePixelRGBAToYACbCr(inRGBAData[pixelIndex], outYA, outCbCr);
			}
		}

		return true;
	}

	bool CreateYACbCrTexture(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, Tex& outTexture)
	{
		if (size.x <= 0 || size.y <= 0)
			return false;

		if (inFormat != TextureFormat::RGBA8)
			return false;

		const auto fullSize = size;
		const auto halfSize = size / 2;

		auto yaBuffer = std::make_unique<u8[]>(fullSize.x * fullSize.y * 2);
		auto fullCbCrBuffer = std::make_unique<u8[]>(fullSize.x * fullSize.y * 2);

		if (!ConvertRGBAToYACbCrBuffer(fullSize, inData, inFormat, inByteSize, yaBuffer.get(), fullCbCrBuffer.get()))
			return false;

		auto fullCbCr = ::DirectX::Image {};
		fullCbCr.width = fullSize.x;
		fullCbCr.height = fullSize.y;
		fullCbCr.format = DXGI_FORMAT_R8G8_UNORM;
		fullCbCr.rowPitch = fullSize.x * 2;
		fullCbCr.slicePitch = fullSize.x * fullSize.y * 2;
		fullCbCr.pixels = fullCbCrBuffer.get();

		auto halfCbCr = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Resize(fullCbCr, halfSize.x, halfSize.y, ::DirectX::TEX_FILTER_LINEAR | ::DirectX::TEX_FILTER_FORCE_NON_WIC, halfCbCr)))
			return false;

		auto halfCbCrSource = ::DirectX::Image {};
		halfCbCrSource.width = halfSize.x;
		halfCbCrSource.height = halfSize.y;
		halfCbCrSource.format = DXGI_FORMAT_R8G8_UNORM;
		halfCbCrSource.rowPitch = halfSize.x * 2;
		halfCbCrSource.slicePitch = halfSize.x * halfSize.y * 2;
		halfCbCrSource.pixels = halfCbCr.GetPixels();

		auto compressedHalfCbCr = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Compress(halfCbCrSource, DXGI_FORMAT_BC5_UNORM, ::DirectX::TEX_COMPRESS_UNIFORM, ::DirectX::TEX_THRESHOLD_DEFAULT, compressedHalfCbCr)))
			return false;

		auto yaSource = ::DirectX::Image {};
		yaSource.width = fullSize.x;
		yaSource.height = fullSize.y;
		yaSource.format = DXGI_FORMAT_R8G8_UNORM;
		yaSource.rowPitch = fullSize.x * 2;
		yaSource.slicePitch = fullSize.x * fullSize.y * 2;
		yaSource.pixels = yaBuffer.get();

		auto compressedYA = ::DirectX::ScratchImage {};
		if (FAILED(::DirectX::Compress(yaSource, DXGI_FORMAT_BC5_UNORM, ::DirectX::TEX_COMPRESS_UNIFORM, ::DirectX::TEX_THRESHOLD_DEFAULT, compressedYA)))
			return false;

		outTexture.MipMapsArray.resize(1);
		auto& mipMaps = outTexture.MipMapsArray.front();

		mipMaps.resize(2);
		auto& mipMapYA = mipMaps[0];
		auto& mipMapCbCr = mipMaps[1];

		mipMapYA.Size = fullSize;
		mipMapCbCr.Size = halfSize;

		for (auto& mip : mipMaps)
		{
			mip.Format = TextureFormat::RGTC2;
			mip.DataSize = static_cast<u32>(TextureFormatByteSize(mip.Size, TextureFormat::RGTC2));
			mip.Data = std::make_unique<u8[]>(mip.DataSize);
		}

		std::memcpy(mipMapYA.Data.get(), compressedYA.GetPixels(), mipMapYA.DataSize);
		std::memcpy(mipMapCbCr.Data.get(), compressedHalfCbCr.GetPixels(), mipMapCbCr.DataSize);

		return true;
	}

	bool ConvertTextureToRGBABuffer(const Tex& inTexture, u8* outData, size_t outByteSize, i32 cubeFace)
	{
		if (inTexture.MipMapsArray.empty() || cubeFace >= inTexture.MipMapsArray.size())
			return false;

		const auto& mips = inTexture.MipMapsArray[cubeFace];
		if (mips.empty())
			return false;

		const auto& frontMip = mips.front();
		const bool decodeYACbCr = (inTexture.GetSignature() == TxpSig::Texture2D && mips.size() == 2 && frontMip.Format == TextureFormat::RGTC2);

		if (decodeYACbCr)
			return ConvertYACbCrToRGBABuffer(mips[0], mips[1], outData, outByteSize);

		return DecompressTextureData(frontMip.Size, frontMip.Data.get(), frontMip.Format, frontMip.DataSize, outData, TextureFormat::RGBA8, outByteSize);
	}

	std::unique_ptr<u8[]> ConvertTextureToRGBA(const Tex& inTexture, i32 cubeFace)
	{
		const auto outByteSize = TextureFormatByteSize(inTexture.GetSize(), TextureFormat::RGBA8);
		auto outData = std::make_unique<u8[]>(outByteSize);

		if (!ConvertTextureToRGBABuffer(inTexture, outData.get(), outByteSize, cubeFace))
			return nullptr;

		return outData;
	}

	bool FlipTextureBufferY(ivec2 size, u8* inOutData, TextureFormat inFormat, size_t inByteSize)
	{
		if (size.x <= 0 || size.y <= 0)
			return false;

		if (inFormat != TextureFormat::RGBA8)
			return false;

		if (inByteSize < TextureFormatByteSize(size, inFormat))
			return false;

		auto inOutRGBAPixels = reinterpret_cast<u32*>(inOutData);

		for (auto y = 0; y < size.y / 2; y++)
		{
			for (auto x = 0; x < size.x; x++)
			{
				u32& pixel = inOutRGBAPixels[size.x * y + x];
				u32& flippedPixel = inOutRGBAPixels[(size.x * (size.y - 1 - y)) + x];

				std::swap(pixel, flippedPixel);
			}
		}

		return true;
	}

	bool LoadDDSToTexture(std::string_view filePath, Tex& outTexture)
	{
		auto outMetadata = ::DirectX::TexMetadata {};
		auto outImage = ::DirectX::ScratchImage {};

		if (FAILED(::DirectX::LoadFromDDSFile(UTF8::WideArg(filePath).c_str(), ::DirectX::DDS_FLAGS_NONE, &outMetadata, outImage)))
			return false;

		outTexture.Name = std::string(IO::Path::GetFileName(filePath, false));
		outTexture.MipMapsArray.resize(outMetadata.arraySize);

		for (size_t arrayIndex = 0; arrayIndex < outTexture.MipMapsArray.size(); arrayIndex++)
		{
			auto& mipMaps = outTexture.MipMapsArray[arrayIndex];
			mipMaps.resize(outMetadata.mipLevels);

			for (size_t mipIndex = 0; mipIndex < mipMaps.size(); mipIndex++)
			{
				const auto* image = outImage.GetImage(mipIndex, arrayIndex, 0);

				auto& mip = mipMaps[mipIndex];
				mip.Size = ivec2(image->width, image->height);
				mip.Format = DXGIFormatToTextureFormat(image->format);

				if (mip.Format == TextureFormat::Unknown)
					return false;

				mip.DataSize = static_cast<u32>(image->slicePitch);
				mip.Data = std::make_unique<u8[]>(mip.DataSize);
				std::memcpy(mip.Data.get(), image->pixels, mip.DataSize);
			}
		}

		return true;
	}

	bool SaveTextureToDDS(std::string_view filePath, const Tex& inTexture)
	{
		if (inTexture.MipMapsArray.empty() || inTexture.MipMapsArray.front().empty())
			return false;

		const auto imageCount = inTexture.MipMapsArray.size() * inTexture.MipMapsArray.front().size();
		if (imageCount < 1)
			return false;

		auto images = std::make_unique<::DirectX::Image[]>(imageCount);
		auto imageWriteHead = images.get();

		for (const auto& mipMaps : inTexture.MipMapsArray)
		{
			for (auto& mip : mipMaps)
			{
				imageWriteHead->width = mip.Size.x;
				imageWriteHead->height = mip.Size.y;
				imageWriteHead->format = TextureFormatToDXGI(mip.Format);
				imageWriteHead->rowPitch = TextureFormatByteSize(ivec2(mip.Size.x, 1), mip.Format);
				imageWriteHead->slicePitch = TextureFormatByteSize(mip.Size, mip.Format);
				imageWriteHead->pixels = const_cast<u8*>(mip.Data.get());
				imageWriteHead++;
			}
		}

		auto metadata = ::DirectX::TexMetadata {};
		metadata.width = images[0].width;
		metadata.height = images[0].height;
		metadata.depth = 1;
		metadata.arraySize = inTexture.MipMapsArray.size();
		metadata.mipLevels = inTexture.MipMapsArray.front().size();
		metadata.format = images[0].format;
		metadata.dimension = ::DirectX::TEX_DIMENSION_TEXTURE2D;

		if (FAILED(::DirectX::SaveToDDSFile(images.get(), imageCount, metadata, ::DirectX::DDS_FLAGS_NONE, UTF8::WideArg(filePath).c_str())))
			return false;

		return true;
	}
}
