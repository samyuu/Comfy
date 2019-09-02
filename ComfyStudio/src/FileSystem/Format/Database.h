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
		String Name;
		int32_t Index;
	};

	struct AetSetEntry : DatabaseEntry, DatabaseFileEntry
	{
		uint32_t ID;
		String Name;
		uint32_t SprSetID;
		Vector<AetEntry> AetEntries;
		String FileName;

		AetEntry* GetAetEntry(const String& name);
	};

	class AetDB : public Database
	{
	public:
		Vector<AetSetEntry> Entries;

		virtual void Read(BinaryReader& reader) override;
		virtual void Write(BinaryWriter& writer) override;
		AetSetEntry* GetAetSetEntry(const String& name);

	private:
	};

	struct SprEntry : DatabaseEntry
	{
		uint32_t ID;
		String Name;
		int16_t Index;
	};

	struct SprSetEntry : DatabaseEntry, DatabaseFileEntry
	{
		uint32_t ID;
		String Name;
		String FileName;

		Vector<SprEntry> SprEntries;
		Vector<SprEntry> SprTexEntries;

		SprEntry* GetSprEntry(uint32_t id);
		SprEntry* GetSprEntry(const String& name);
		SprEntry* GetSprTexEntry(const String& name);
	};

	class SprDB : public Database
	{
	public:
		Vector<SprSetEntry> Entries;

		virtual void Read(BinaryReader& reader) override;
		virtual void Write(BinaryWriter& writer) override;
		SprSetEntry* GetSprSetEntry(const String& name);

		uint32_t GetSprSetEntryCount();
		uint32_t GetSprEntryCount();

	private:
	};
}