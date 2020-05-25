#pragma once
#include "IO/Archive/ComfyArchive.h"
#include "IO/Config/ComfyBinaryConfig.h"

namespace Comfy::System
{
	constexpr std::string_view DataFileName = "ComfyData.bin";
	constexpr std::string_view ConfigFileName = "ComfyConfig.bin";

	// TODO: Make sure the program is able ot start without any of these present even if that means it is rendered unusable
	extern IO::ComfyArchive Data;
	extern IO::ComfyBinaryConfig Config;

	// NOTE: Should be called some time before the ApplicationHost is created
	void MountComfyData();
	void UnMountComfyData();

	// NOTE: Up to the child application how this is used, i.e. restore last window position
	void LoadComfyConfig();
	void SaveComfyConfig();
}
