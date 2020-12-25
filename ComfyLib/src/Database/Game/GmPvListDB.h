#pragma once
#include "Database/Database.h"

namespace Comfy::Database
{
	struct GmPvDifficultyEntry
	{
		DateEntry StartDate, EndDate;
		i32 Edition;
		i32 Version;
	};

	struct GmPvDifficultyEntries
	{
		std::array<GmPvDifficultyEntry, 2> Editions;
		i32 Count;
	};

	struct GmPvEntry
	{
		DateEntry AdvDemoStartDate, AdvDemoEndDate;
		GmPvDifficultyEntries Easy, Normal, Hard, Extreme, Encore;
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
