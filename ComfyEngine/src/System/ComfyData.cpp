#include "ComfyData.h"
#include "IO/File.h"
#include "Core/Logger.h"

namespace Comfy::System
{
	IO::ComfyArchive Data;
	IO::ComfyBinaryConfig Config;

	bool DataMounted = false;
	bool ConfigLoaded = false;

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

	void LoadComfyConfig()
	{
		if (ConfigLoaded)
			return;
		ConfigLoaded = true;

		auto stream = IO::File::OpenReadMemory(ConfigFileName);
		if (!stream.IsOpen() || !stream.CanRead())
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unable to open config file");
			return;
		}

		auto reader = IO::StreamReader(stream);
		Config.Read(reader);
	}

	void SaveComfyConfig()
	{
		if (!ConfigLoaded)
			return;

		if (!IO::File::Save(ConfigFileName, Config))
			Logger::LogErrorLine(__FUNCTION__"(): Unable to save config file");
	}
}
