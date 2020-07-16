#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/TexSet.h"

namespace Comfy::Graphics::Utilities
{
	COMFY_NODISCARD constexpr u32 RoundToNearestPowerOfTwo(u32 input)
	{
		input--;
		input |= input >> 1;
		input |= input >> 2;
		input |= input >> 4;
		input |= input >> 8;
		input |= input >> 16;
		input++;
		return input;
	}

	COMFY_NODISCARD constexpr ivec2 RoundToNearestPowerOfTwo(ivec2 input)
	{
		return ivec2(
			static_cast<i32>(RoundToNearestPowerOfTwo(static_cast<u32>(std::max(input.x, 1)))),
			static_cast<i32>(RoundToNearestPowerOfTwo(static_cast<u32>(std::max(input.y, 1)))));
	}

	COMFY_NODISCARD size_t TextureFormatBlockSize(TextureFormat format);
	COMFY_NODISCARD size_t TextureFormatChannelCount(TextureFormat format);
	COMFY_NODISCARD size_t TextureFormatByteSize(ivec2 size, TextureFormat format);

	// NOTE: Raw decompression routine, the output format must not be compressed
	COMFY_NODISCARD bool DecompressTextureData(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outData, TextureFormat outFormat, size_t outByteSize);

	// NOTE: Raw compression routine, the input format must not be compressed
	//		 While reasonably fast to compute, the output is not quite as high quallity as that of the slow NVTT compression
	COMFY_NODISCARD bool CompressTextureData(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outData, TextureFormat outFormat, size_t outByteSize);

	// NOTE: For internal use, usually shouldn't be called on its own.
	bool ConvertYACbCrToRGBABuffer(const TexMipMap& mipMapYA, const TexMipMap& mipMapCbCr, u8* outData, size_t outByteSize);

	// NOTE: For internal use, usually shouldn't be called on its own. Both the YA and CbCr buffers are expected to be the same size
	bool ConvertRGBAToYACbCrBuffer(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outYAData, u8* outCbCrData);

	bool CreateYACbCrTexture(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, Tex& outTexture);

	// NOTE: Includes automatic checking and decoding of YACbCr RGTC2 textures
	COMFY_NODISCARD bool ConvertTextureToRGBABuffer(const Tex& inTexture, u8* outData, size_t outByteSize, i32 cubeFace = 0);
	COMFY_NODISCARD std::unique_ptr<u8[]> ConvertTextureToRGBA(const Tex& inTexture, i32 cubeFace = 0);

	// NOTE: In place texture flip, in most cases it's probably more optimal to flip during reading or writing of the pixel data instead
	bool FlipTextureBufferY(ivec2 size, u8* inOutData, TextureFormat inFormat, size_t inByteSize);

	bool ConvertRGBToRGBA(ivec2 size, const u8* inData, size_t inByteSize, u8* outData, size_t outByteSize);

	bool LoadDDSToTexture(std::string_view filePath, Tex& outTexture);
	bool SaveTextureToDDS(std::string_view filePath, const Tex& inTexture);
}
