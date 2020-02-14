#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Utilities
{
	void WritePNG(std::string_view filePath, ivec2 size, const uint8_t* rgbaPixels);
}
