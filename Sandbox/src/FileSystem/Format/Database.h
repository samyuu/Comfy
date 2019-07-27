#pragma once
#include "FileSystem/FileInterface.h"
#include "Types.h"
#include <string>
#include <vector>

namespace FileSystem
{
	class Database : public IBinaryReadable
	{
	};
	
	struct DatabaseEntry
	{
		uint32_t ID;
		std::string Name;
	};

	struct DatabaseFileEntry
	{
		std::string FileName;
	};

	struct AetEntry : DatabaseEntry
	{
	};

	struct AetSetEntry : DatabaseEntry, DatabaseFileEntry
	{
		uint32_t SprSetID;
		std::vector<AetEntry> AetEntries;

		AetEntry* GetAetEntry(const std::string& name);
	};

	class AetDB : public Database
	{
	public:
		std::vector<AetSetEntry> Entries;

		virtual void Read(BinaryReader& reader) override;
		AetSetEntry* GetAetSetEntry(const std::string& name);

	private:
	};

	struct SprEntry : DatabaseEntry
	{
	};

	struct SprSetEntry : DatabaseEntry, DatabaseFileEntry
	{
		std::vector<SprEntry> SprEntries;
		std::vector<SprEntry> SprTexEntries;

		SprEntry* GetSprEntry(const std::string& name);
		SprEntry* GetSprTexEntry(const std::string& name);
	};

	class SprDB : public Database
	{
	public:
		std::vector<SprSetEntry> Entries;

		virtual void Read(BinaryReader& reader) override;
		SprSetEntry* GetSprSetEntry(const std::string& name);

	private:
	};
}