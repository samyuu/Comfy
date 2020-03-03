#pragma once
#include "Database.h"

namespace Comfy::Database
{
	struct AetEntry : BinaryDatabase::Entry
	{
		AetID ID;
		std::string Name;
		int32_t Index;
	};

	struct AetSetEntry : BinaryDatabase::Entry, BinaryDatabase::FileEntry
	{
		AetSetID ID;
		std::string Name;
		SprSetID SprSetID;
		std::vector<AetEntry> AetEntries;
		std::string FileName;

		AetEntry* GetAetEntry(const std::string& name);
	};

	class AetDB final : public BinaryDatabase
	{
	public:
		std::vector<AetSetEntry> Entries;

		void Read(FileSystem::BinaryReader& reader) override;
		void Write(FileSystem::BinaryWriter& writer) override;
		AetSetEntry* GetAetSetEntry(std::string_view name);

	private:
	};
}
