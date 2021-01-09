#pragma once
#include "Database/Database.h"
#include "Script/PVScript.h"
#include "Time/TimeSpan.h"

namespace Comfy::Database
{
	enum class PVDifficultyType : i32
	{
		Easy,
		Normal,
		Hard,
		Extreme,
		Encore,
		Count
	};

	enum class PVDifficultyLevel
	{
		Level_00_0, Level_00_5,
		Level_01_0, Level_01_5,
		Level_02_0, Level_02_5,
		Level_03_0, Level_03_5,
		Level_04_0, Level_04_5,
		Level_05_0, Level_05_5,
		Level_06_0, Level_06_5,
		Level_07_0, Level_07_5,
		Level_08_0, Level_08_5,
		Level_09_0, Level_09_5,
		Level_10_0,
		Count,
	};

	constexpr std::array<const char*, EnumCount<PVDifficultyLevel>()> PVDifficultyLevelNames =
	{
		"PV_LV_00_0", "PV_LV_00_5",
		"PV_LV_01_0", "PV_LV_01_5",
		"PV_LV_02_0", "PV_LV_02_5",
		"PV_LV_03_0", "PV_LV_03_5",
		"PV_LV_04_0", "PV_LV_04_5",
		"PV_LV_05_0", "PV_LV_05_5",
		"PV_LV_06_0", "PV_LV_06_5",
		"PV_LV_07_0", "PV_LV_07_5",
		"PV_LV_08_0", "PV_LV_08_5",
		"PV_LV_09_0", "PV_LV_09_5",
		"PV_LV_10_0",
	};

	enum class PVDifficultyEdition : i32
	{
		Normal,
		Extra,
		Count
	};

	struct PvDBAetSceneLayerEntry
	{
		std::string Layer;
		std::string Scene;
	};

	class PvDB final : public TextDatabase, public IO::IStreamWritable
	{
	public:

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
	};
}
