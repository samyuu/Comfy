#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Util
{
	void ReadImage(std::string_view filePath, ivec2& outSize, std::unique_ptr<u8[]>& outRGBAPixels);
	void WritePNG(std::string_view filePath, ivec2 size, const void* rgbaPixels);
}
