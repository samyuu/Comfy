#pragma once
#include "FileSystem/Archive/ComfyArchive.h"

using ComfyArchive = FileSystem::ComfyArchive;

constexpr const char* ComfyDataFileName = "ComfyData.bin";

extern UniquePtr<ComfyArchive> ComfyData;