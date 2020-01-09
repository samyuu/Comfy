#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"

namespace Editor
{
	enum class StageType 
	{ 
		STGTST,
		STGNS, 
		STGD2NS, 
		STGPV, 
		Count 
	};

	struct StageTestData
	{
		struct StageTypeData
		{
			const StageType Type;
			const char* Name;
			const int MinID, MaxID;
			int ID;
			int SubID;
		};

		std::array<StageTypeData, 4> TypeData =
		{
			StageTypeData { StageType::STGTST, "STGTST", 0, 10, 7, -1 },
			StageTypeData { StageType::STGNS, "STGNS", 1, 292, 1, -1 },
			StageTypeData { StageType::STGD2NS, "STGD2NS", 35, 82, 35, -1 },
			StageTypeData { StageType::STGPV, "STGPV", 1, 999, 1, 1 },
		};

		struct Settings
		{
			bool SelectGround = false;
			bool LoadLightParam = true;
			bool LoadObj = true;
		} Settings;

	};
}
