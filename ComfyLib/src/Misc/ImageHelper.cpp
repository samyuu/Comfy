#include "ImageHelper.h"
#include "IO/Path.h"
#include "Misc/StringUtil.h"

#include <zlib.h>
#define STBIW_MALLOC(sz)        malloc(sz)
#define STBIW_REALLOC(p,newsz)  realloc(p,newsz)
#define STBIW_FREE(p)           free(p)
#define STBIW_WINDOWS_UTF8
#define STB_IMAGE_WRITE_IMPLEMENTATION

namespace
{
	unsigned char* CustomStbImageZLibCompress2(const unsigned char* inData, int inDataSize, int* outDataSize, int inQuality)
	{
		// NOTE: If successful this buffer will be freed by stb image
		auto outBuffer = static_cast<unsigned char*>(STBIW_MALLOC(inDataSize));

		uLongf compressedSize = inDataSize;
		int compressResult = compress2(outBuffer, &compressedSize, inData, inDataSize, inQuality);

		*outDataSize = static_cast<int>(compressedSize);

		if (compressResult != Z_OK)
		{
			STBIW_FREE(outBuffer);
			return nullptr;
		}

		return outBuffer;
	}
}

#define STBIW_ZLIB_COMPRESS CustomStbImageZLibCompress2
#include <stb_image_write.h>

// #define STBI_NO_JPEG
// #define STBI_NO_PNG
// #define STBI_NO_BMP
// #define STBI_NO_PSD
#define STBI_NO_TGA
// #define STBI_NO_GIF
#define STBI_NO_HDR
#define STBI_NO_PIC
#define STBI_NO_PNM
#define STBI_WINDOWS_UTF8
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Comfy::Util
{
	void ReadImage(std::string_view filePath, ivec2& outSize, std::unique_ptr<u8[]>& outRGBAPixels)
	{
		constexpr int rgbaComponents = 4;

		int components;
		stbi_uc* pixels = stbi_load(filePath.data(), &outSize.x, &outSize.y, &components, rgbaComponents);

		if (pixels != nullptr)
		{
			const size_t dataSize = (outSize.x * outSize.y * rgbaComponents);
			outRGBAPixels = std::make_unique<u8[]>(dataSize);
			std::memcpy(outRGBAPixels.get(), pixels, dataSize);
		}

		stbi_image_free(pixels);
	}

	void WriteImage(std::string_view filePath, ivec2 size, const void* rgbaPixels)
	{
		if (rgbaPixels == nullptr || size.x <= 0 || size.y <= 0)
			return;

		constexpr int channelCount = 4;

		const auto extension = IO::Path::GetExtension(filePath);
		const auto nullTerminatedFilePath = std::string(filePath);

		if (Util::MatchesInsensitive(extension, ".bmp"))
		{
			stbi_write_bmp(nullTerminatedFilePath.data(), size.x, size.y, channelCount, rgbaPixels);
		}
		else if (Util::MatchesInsensitive(extension, ".tga"))
		{
			stbi_write_tga(nullTerminatedFilePath.data(), size.x, size.y, channelCount, rgbaPixels);
		}
		else
		{
			// constexpr int bitsPerPixel = channelCount * CHAR_BIT;
			// const auto pixelsPerLine = (size.x % bitsPerPixel != 0) ? (size.x + (bitsPerPixel - (size.x % bitsPerPixel))) : (size.x);

			// NOTE: For simplicity sake proper 16 byte stride alignment will be ignored for now
			const auto pixelsPerLine = size.x;

			stbi_write_png(nullTerminatedFilePath.data(), size.x, size.y, channelCount, rgbaPixels, pixelsPerLine * channelCount);
		}
	}
}
