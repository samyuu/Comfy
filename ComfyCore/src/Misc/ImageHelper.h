#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Utilities
{
	void WritePNG(std::string_view filePath, ivec2 size, const void* rgbaPixels);
}
