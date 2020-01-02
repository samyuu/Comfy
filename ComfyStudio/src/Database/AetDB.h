#pragma once
#include "Database.h"

namespace Database
{
	struct AetEntry : BinaryDatabase::Entry
	{
		uint32_t ID;
		std::string Name;
		int32_t Index;
	};

	struct AetSetEntry : BinaryDatabase::Entry, BinaryDatabase::FileEntry
	{
		uint32_t ID;
		std::string Name;
		uint32_t SprSetID;
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
		AetSetEntry* GetAetSetEntry(const std::string& name);

	private:
	};
}
