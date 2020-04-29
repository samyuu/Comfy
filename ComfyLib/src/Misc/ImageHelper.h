#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Utilities
{
	void ReadImage(std::string_view filePath, ivec2& outSize, UniquePtr<u8[]>& outRGBAPixels);
	void WritePNG(std::string_view filePath, ivec2 size, const void* rgbaPixels);
}
