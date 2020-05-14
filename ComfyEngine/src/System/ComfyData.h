#pragma once
#include "IO/Archive/ComfyArchive.h"

namespace Comfy
{
	constexpr std::string_view ComfyDataFileName = "ComfyData.bin";

	inline std::unique_ptr<IO::ComfyArchive> ComfyData = nullptr;
}
