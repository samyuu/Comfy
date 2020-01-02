#pragma once
#include "Database.h"

namespace Database
{
	struct AetEntry : DatabaseEntry
	{
		uint32_t ID;
		std::string Name;
		int32_t Index;
	};

	struct AetSetEntry : DatabaseEntry, DatabaseFileEntry
	{
		uint32_t ID;
		std::string Name;
		uint32_t SprSetID;
		std::vector<AetEntry> AetEntries;
		std::string FileName;

		AetEntry* GetAetEntry(const std::string& name);
	};

	class AetDB final : public Database
	{
	public:
		std::vector<AetSetEntry> Entries;

		void Read(FileSystem::BinaryReader& reader) override;
		void Write(FileSystem::BinaryWriter& writer) override;
		AetSetEntry* GetAetSetEntry(const std::string& name);

	private:
	};
}
