#include "ComfyData.h"
#include "IO/File.h"
#include "Core/Logger.h"

namespace Comfy::System
{
	IO::ComfyArchive Data;
	static bool DataMounted = false;

	void MountComfyData()
	{
		if (DataMounted)
			return;
		DataMounted = true;

		if (!IO::File::Exists(DataFileName))
			Logger::LogErrorLine(__FUNCTION__"(): Unable to locate data file");

		if (!Data.Mount(DataFileName))
			Logger::LogErrorLine(__FUNCTION__"(): Unable to mount data file");
	}

	void UnMountComfyData()
	{
		if (!DataMounted)
			return;

		Data.UnMount();
	}
}
