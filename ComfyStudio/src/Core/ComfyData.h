#pragma once
#include "IO/Archive/ComfyArchive.h"

namespace Comfy
{
	constexpr std::string_view ComfyDataFileName = "ComfyData.bin";

	inline UniquePtr<IO::ComfyArchive> ComfyData = nullptr;
}
