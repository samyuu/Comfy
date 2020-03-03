#include "AetDB.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"

namespace Comfy::Database
{
	using namespace FileSystem;

	AetEntry* AetSetEntry::GetAetEntry(const std::string& name)
	{
		for (auto& entry : AetEntries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}

	void AetDB::Read(BinaryReader& reader)
	{
		uint32_t aetSetEntryCount = reader.ReadU32();
		FileAddr aetSetOffset = reader.ReadPtr();

		uint32_t aetEntryCount = reader.ReadU32();
		FileAddr aetOffset = reader.ReadPtr();

		if (aetSetEntryCount > 0 && aetSetOffset != FileAddr::NullPtr)
		{
			Entries.resize(aetSetEntryCount);
			reader.ReadAt(aetSetOffset, [this](BinaryReader& reader)
			{
				for (auto& aetSetEntry : Entries)
				{
					aetSetEntry.ID = AetSetID(reader.ReadU32());
					aetSetEntry.Name = reader.ReadStrPtr();
					aetSetEntry.FileName = reader.ReadStrPtr();
					uint32_t index = reader.ReadI32();
					aetSetEntry.SprSetID = SprSetID(reader.ReadU32());
				}
			});
		}

		if (aetEntryCount > 0 && aetOffset != FileAddr::NullPtr)
		{
			reader.ReadAt(aetOffset, [this, aetEntryCount](BinaryReader& reader)
			{
				for (uint32_t i = 0; i < aetEntryCount; i++)
				{
					AetID id = AetID(reader.ReadU32());
					FileAddr nameOffset = reader.ReadPtr();
					int16_t aetIndex = reader.ReadI16();
					int16_t setIndex = reader.ReadI16();

					AetSetEntry& aetSetEntry = Entries[setIndex];

					aetSetEntry.AetEntries.emplace_back();
					AetEntry& aetEntry = aetSetEntry.AetEntries.back();

					aetEntry.ID = id;
					aetEntry.Name = reader.ReadStrAt(nameOffset);
				}
			});
		}
	}

	AetSetEntry* AetDB::GetAetSetEntry(std::string_view name)
	{
		for (auto& entry : Entries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}
}
