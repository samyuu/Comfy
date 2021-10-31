#pragma once
#include "Types.h"
#include "Graphics/Auth2D/SprSet.h"

namespace Comfy::Graphics::Utilities
{
	void ExtractAllSprPNGs(std::string_view outputDirectory, const SprSet& sprSet);
}