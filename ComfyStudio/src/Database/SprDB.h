#pragma once
#include "Database.h"

namespace Database
{
	struct SprEntry : DatabaseEntry
	{
		uint32_t ID;
		std::string Name;
		int16_t Index;
	};

	struct SprSetEntry : DatabaseEntry, DatabaseFileEntry
	{
		uint32_t ID;
		std::string Name;
		std::string FileName;

		std::vector<SprEntry> SprEntries;
		std::vector<SprEntry> SprTexEntries;

		SprEntry* GetSprEntry(uint32_t id);
		SprEntry* GetSprEntry(const std::string& name);
		SprEntry* GetSprTexEntry(const std::string& name);
	};

	class SprDB final : public Database
	{
	public:
		std::vector<SprSetEntry> Entries;

		void Read(FileSystem::BinaryReader& reader) override;
		void Write(FileSystem::BinaryWriter& writer) override;
		SprSetEntry* GetSprSetEntry(const std::string& name);

		uint32_t GetSprSetEntryCount();
		uint32_t GetSprEntryCount();

	private:
	};
}
