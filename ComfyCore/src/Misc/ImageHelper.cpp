#include "ImageHelper.h"

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

namespace Utilities
{
	void WritePNG(std::string_view filePath, ivec2 size, const uint8_t* rgbaPixels)
	{
		// TODO: Handle case of filePath not being null terminated
		assert(rgbaPixels != nullptr && size.x > 0 && size.y > 0);

		constexpr int channels = 4;
		constexpr int bitsPerPixel = channels * CHAR_BIT;

		const auto pixelsPerLine = (size.x % bitsPerPixel != 0) ? (size.x + (bitsPerPixel - (size.x % bitsPerPixel))) : (size.x);
		stbi_write_png(filePath.data(), size.x, size.y, channels, rgbaPixels, pixelsPerLine * channels);
	}
}
