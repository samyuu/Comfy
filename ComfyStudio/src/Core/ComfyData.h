#pragma once
#include "FileSystem/Archive/ComfyArchive.h"

using ComfyArchive = FileSystem::ComfyArchive;

constexpr std::string_view ComfyDataFileName = "ComfyData.bin";

extern UniquePtr<ComfyArchive> ComfyData;