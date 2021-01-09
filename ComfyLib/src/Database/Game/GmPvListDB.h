#pragma once
#include "Database/Database.h"
#include "PvDB.h"

namespace Comfy::Database
{
	struct GmPvDifficultyEntry
	{
		DateEntry StartDate, EndDate;
		PVDifficultyEdition Edition;
		i32 Version;
	};

	struct GmPvDifficultyEditionsEntry
	{
		std::array<GmPvDifficultyEntry, EnumCount<PVDifficultyEdition>()> Editions;
		i32 Count;
	};

	struct GmPvEntry
	{
		DateEntry AdvDemoStartDate, AdvDemoEndDate;
		std::array<GmPvDifficultyEditionsEntry, EnumCount<PVDifficultyType>()> Difficulties;
		std::string Name;
		i32 ID;
		i32 Ignore;
	};

	class GmPvListDB final : public TextDatabase, public IO::IStreamWritable
	{
	public:
		std::vector<GmPvEntry> Entries;

	public:
		void Parse(const u8* buffer, size_t bufferSize) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
	};
}
