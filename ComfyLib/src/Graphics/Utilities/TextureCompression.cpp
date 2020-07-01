#include "TextureCompression.h"
#include "Core/Win32/ComfyWindows.h"
#include <DirectXTex.h>

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
	}

	size_t GetTextureFormatBlockSize(TextureFormat format)
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

	size_t GetTextureFormatChannelCount(TextureFormat format)
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

	size_t GetTextureFormatByteSize(ivec2 size, TextureFormat format)
	{
		switch (format)
		{
		case TextureFormat::A8:
		case TextureFormat::L8:
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
		case TextureFormat::L8A8:
			return size.x * size.y * GetTextureFormatChannelCount(format);

			// BUG: Not entirely accurate but not like these are used anyway
		case TextureFormat::RGB5:
			assert(false);
			return (size.x * size.y * 15) / CHAR_BIT;
		case TextureFormat::RGB5_A1:
			assert(false);
			return (size.x * size.y * 16) / CHAR_BIT;

		case TextureFormat::RGBA4:
			return (size.x * size.y * GetTextureFormatChannelCount(format)) / 2;

		case TextureFormat::DXT1:
		case TextureFormat::DXT1a:
		case TextureFormat::DXT3:
		case TextureFormat::DXT5:
		case TextureFormat::RGTC1:
		case TextureFormat::RGTC2:
			return glm::max(1, (size.x + 3) / 4) * glm::max(1, (size.y + 3) / 4) * GetTextureFormatBlockSize(format);

		default:
			assert(false);
			return 0;
		}
	}

	bool DecompressTextureFormat(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outData, TextureFormat outFormat, size_t outByteSize)
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
		inputImage.rowPitch = GetTextureFormatByteSize(ivec2(size.x, 1), inFormat);
		inputImage.slicePitch = GetTextureFormatByteSize(ivec2(1, size.y), inFormat);
		inputImage.pixels = const_cast<u8*>(inData);

		auto outputImage = ::DirectX::ScratchImage {};

		const auto decompressResult = ::DirectX::Decompress(inputImage, outFormatDXGI, outputImage);
		if (FAILED(decompressResult))
			return false;

		if (outByteSize < outputImage.GetPixelsSize())
			return false;

		std::memcpy(outData, outputImage.GetPixels(), outputImage.GetPixelsSize());
		return true;
	}

	namespace
	{
		constexpr vec3 RED_COEF_709 = { +1.5748f, +1.0f, +0.0000f };
		constexpr vec3 GRN_COEF_709 = { -0.4681f, +1.0f, -0.1873f };
		constexpr vec3 BLU_COEF_709 = { +0.0000f, +1.0f, +1.8556f };

		constexpr float OFFSET_709 = 0.503929f;
		constexpr float FACTOR_709 = 1.003922f;

		constexpr __forceinline float PixelU8ToF32(u8 pixel)
		{
			constexpr auto factor = 1.0f / static_cast<float>(std::numeric_limits<u8>::max());
			return static_cast<float>(pixel) * factor;
		}

		constexpr __forceinline u8 PixelF32ToU8(float pixel)
		{
			constexpr auto factor = static_cast<float>(std::numeric_limits<u8>::max());

			const auto clamped = std::clamp(pixel, 0.0f, 1.0f);
			return static_cast<u8>(clamped * factor);
		}

		constexpr __forceinline u32 PackU8RGBA(u8 r, u8 g, u8 b, u8 a)
		{
			return
				(static_cast<u32>(a) << 24) |
				(static_cast<u32>(b) << 16) |
				(static_cast<u32>(g) << 8) |
				(static_cast<u32>(r));
		}

		bool ConvertYACbCrToRGBA(const TexMipMap& mipMapYA, const TexMipMap& mipMapCbCr, u8* outData, size_t outByteSize)
		{
			if (mipMapYA.Format != TextureFormat::RGTC2 || mipMapCbCr.Format != TextureFormat::RGTC2 || (mipMapYA.Size / 2) != mipMapCbCr.Size)
				return false;

			if (outByteSize < GetTextureFormatByteSize(mipMapYA.Size, TextureFormat::RGBA8))
				return false;

			auto imageFromMip = [](const auto& mip)
			{
				auto image = ::DirectX::Image {};
				image.width = mip.Size.x;
				image.height = mip.Size.y;
				image.format = DXGI_FORMAT_BC5_UNORM;
				image.rowPitch = GetTextureFormatByteSize(ivec2(mip.Size.x, 1), TextureFormat::RGTC2);
				image.slicePitch = GetTextureFormatByteSize(ivec2(1, mip.Size.y), TextureFormat::RGTC2);
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
					const auto pixelIndex = (y * mipMapYA.Size.x + x) * 2;
					const u8* pixelsYA = &pixelBufferYA[pixelIndex];
					const u8* pixelsCbCr = &pixelBufferCbCrResized[pixelIndex];

					const vec2 cbCr = vec2(PixelU8ToF32(pixelsCbCr[0]), PixelU8ToF32(pixelsCbCr[1])) * FACTOR_709 - OFFSET_709;
					const vec3 mixed = vec3(cbCr[1], PixelU8ToF32(pixelsYA[0]), cbCr[0]);

					outRGBA[y * mipMapYA.Size.x + x] = PackU8RGBA(
						PixelF32ToU8(glm::dot(mixed, RED_COEF_709)),
						PixelF32ToU8(glm::dot(mixed, GRN_COEF_709)),
						PixelF32ToU8(glm::dot(mixed, BLU_COEF_709)),
						pixelsYA[1]);
				}
			}

			return true;
		}
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
			return ConvertYACbCrToRGBA(mips[0], mips[1], outData, outByteSize);

		return DecompressTextureFormat(frontMip.Size, frontMip.Data.get(), frontMip.Format, frontMip.DataSize, outData, TextureFormat::RGBA8, outByteSize);
	}

	std::unique_ptr<u8[]> ConvertTextureToRGBA(const Tex& inTexture, i32 cubeFace)
	{
		const auto outByteSize = GetTextureFormatByteSize(inTexture.GetSize(), TextureFormat::RGBA8);
		auto outData = std::make_unique<u8[]>(outByteSize);

		if (!ConvertTextureToRGBABuffer(inTexture, outData.get(), outByteSize, cubeFace))
			return nullptr;

		return outData;
	}
}
