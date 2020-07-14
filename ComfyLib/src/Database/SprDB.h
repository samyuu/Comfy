#pragma once
#include "Database.h"

namespace Comfy::Database
{
	struct SprEntry : BinaryDatabase::Entry
	{
		SprID ID;
		std::string Name;
		i16 Index;
	};

	struct SprSetEntry : BinaryDatabase::Entry, BinaryDatabase::FileEntry
	{
		SprSetID ID;
		std::string Name;
		std::string FileName;

		std::vector<SprEntry> SprEntries;
		std::vector<SprEntry> SprTexEntries;

		SprEntry* GetSprEntry(SprID id);
		SprEntry* GetSprEntry(std::string_view name);
		SprEntry* GetSprTexEntry(std::string_view name);
	};

	class SprDB final : public BinaryDatabase
	{
	public:
		std::vector<SprSetEntry> Entries;

		IO::StreamResult Read(IO::StreamReader& reader) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

		SprSetEntry* GetSprSetEntry(std::string_view name);

		u32 GetSprSetEntryCount();
		u32 GetSprEntryCount();

	private:
	};
}
