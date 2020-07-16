#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Util
{
	bool ReadImage(std::string_view filePath, ivec2& outSize, std::unique_ptr<u8[]>& outRGBAPixels);
	bool WriteImage(std::string_view filePath, ivec2 size, const void* rgbaPixels);
}
