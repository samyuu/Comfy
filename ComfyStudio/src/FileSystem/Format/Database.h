#pragma once
#include "FileSystem/FileInterface.h"
#include "Types.h"

namespace FileSystem
{
	class Database : public IBinaryReadable, public IBinaryWritable
	{
	};
	
	struct DatabaseEntry
	{
	};

	struct DatabaseFileEntry
	{
	};

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

	class AetDB : public Database
	{
	public:
		std::vector<AetSetEntry> Entries;

		virtual void Read(BinaryReader& reader) override;
		virtual void Write(BinaryWriter& writer) override;
		AetSetEntry* GetAetSetEntry(const std::string& name);

	private:
	};

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

	class SprDB : public Database
	{
	public:
		std::vector<SprSetEntry> Entries;

		virtual void Read(BinaryReader& reader) override;
		virtual void Write(BinaryWriter& writer) override;
		SprSetEntry* GetSprSetEntry(const std::string& name);

		uint32_t GetSprSetEntryCount();
		uint32_t GetSprEntryCount();

	private:
	};
}