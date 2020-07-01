#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/TexSet.h"

namespace Comfy::Graphics::Utilities
{
	COMFY_NODISCARD size_t GetTextureFormatBlockSize(TextureFormat format);
	COMFY_NODISCARD size_t GetTextureFormatChannelCount(TextureFormat format);
	COMFY_NODISCARD size_t GetTextureFormatByteSize(ivec2 size, TextureFormat format);

	COMFY_NODISCARD bool DecompressTextureFormat(ivec2 size, const u8* inData, TextureFormat inFormat, size_t inByteSize, u8* outData, TextureFormat outFormat, size_t outByteSize);

	COMFY_NODISCARD bool ConvertTextureToRGBABuffer(const Tex& inTexture, u8* outData, size_t outByteSize, i32 cubeFace = 0);
	COMFY_NODISCARD std::unique_ptr<u8[]> ConvertTextureToRGBA(const Tex& inTexture, i32 cubeFace = 0);
}
