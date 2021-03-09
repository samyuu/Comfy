#pragma once
#include "IO/Archive/ComfyArchive.h"

namespace Comfy::System
{
	constexpr std::string_view DataFileName = "ComfyData.dat";

	// NOTE: Always make sure the program is able ot start without any of these present even if that means it is rendered unusable
	extern IO::ComfyArchive Data;

	// NOTE: Should be called some time before the ApplicationHost is created
	void MountComfyData();
	void UnMountComfyData();
}
