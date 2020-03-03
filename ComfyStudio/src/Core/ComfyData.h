#pragma once
#include "FileSystem/Archive/ComfyArchive.h"

namespace Comfy
{
	using ComfyArchive = FileSystem::ComfyArchive;

	constexpr std::string_view ComfyDataFileName = "ComfyData.bin";

	inline UniquePtr<ComfyArchive> ComfyData = nullptr;
}
