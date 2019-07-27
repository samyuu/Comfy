#include "Database.h"
#include "FileSystem/BinaryReader.h"

namespace FileSystem
{
	AetEntry* AetSetEntry::GetAetEntry(const std::string& name)
	{
		return nullptr;
	}

	void AetDB::Read(BinaryReader& reader)
	{
		uint32_t aetSetEntryCount = reader.ReadUInt32();
		void* aetSetOffset = reader.ReadPtr();

		uint32_t aetEntryCount = reader.ReadUInt32();
		void* aetOffset = reader.ReadPtr();

		if (aetSetEntryCount > 0 && aetSetOffset != nullptr)
		{
			Entries.resize(aetSetEntryCount);
			reader.ReadAt(aetSetOffset, [this](BinaryReader& reader)
			{
				for (auto& aetSetEntry : Entries)
				{
					aetSetEntry.ID = reader.ReadUInt32();
					aetSetEntry.Name = reader.ReadStrPtr();
					aetSetEntry.FileName = reader.ReadStrPtr();
					uint32_t index = reader.ReadInt32();
					aetSetEntry.SprSetID = reader.ReadUInt32();
				}
			});
		}

		if (aetEntryCount > 0 && aetOffset != nullptr)
		{
			reader.ReadAt(aetOffset, [this, aetEntryCount](BinaryReader& reader)
			{
				for (uint32_t i = 0; i < aetEntryCount; i++)
				{
					uint32_t id = reader.ReadUInt32();
					void* nameOffset = reader.ReadPtr();
					int16_t aetIndex = reader.ReadInt16();
					int16_t setIndex = reader.ReadInt16();

					AetSetEntry& aetSetEntry = Entries[setIndex];

					aetSetEntry.AetEntries.emplace_back();
					AetEntry& aetEntry = aetSetEntry.AetEntries.back();

					aetEntry.ID = id;
					aetEntry.Name = reader.ReadStr(nameOffset);
				}
			});
		}
	}

	AetSetEntry* AetDB::GetAetSetEntry(const std::string& name)
	{
		for (auto& entry : Entries) 
			if (entry.Name == name) 
				return &entry;
		return nullptr;
	}

	SprEntry* SprSetEntry::GetSprEntry(const std::string& name)
	{
		for (auto& entry : SprEntries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}

	SprEntry* SprSetEntry::GetSprTexEntry(const std::string& name)
	{
		for (auto& entry : SprTexEntries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}

	void SprDB::Read(BinaryReader& reader)
	{
		uint32_t sprSetEntryCount = reader.ReadUInt32();
		void* sprSetOffset = reader.ReadPtr();

		uint32_t sprEntryCount = reader.ReadUInt32();
		void* sprOffset = reader.ReadPtr();

		if (sprSetEntryCount > 0 && sprSetOffset != nullptr)
		{
			Entries.resize(sprSetEntryCount);
			reader.ReadAt(sprSetOffset, [this](BinaryReader& reader)
			{
				for (auto& sprSetEntry : Entries)
				{
					sprSetEntry.ID = reader.ReadUInt32();
					sprSetEntry.Name = reader.ReadStrPtr();
					sprSetEntry.FileName = reader.ReadStrPtr();
					uint32_t index = reader.ReadInt32();
				}
			});
		}

		if (sprEntryCount > 0 && sprOffset != nullptr)
		{
			reader.ReadAt(sprOffset, [this, sprEntryCount](BinaryReader& reader)
			{
				for (uint32_t i = 0; i < sprEntryCount; i++)
				{
					constexpr uint16_t packedDataMask = 0x1000;

					uint32_t id = reader.ReadUInt32();
					void* nameOffset = reader.ReadPtr();
					int16_t index = reader.ReadInt16();
					uint16_t packedData = reader.ReadUInt16();

					int32_t sprSetEntryIndex = (packedData & ~packedDataMask);
					SprSetEntry& sprSetEntry = Entries[sprSetEntryIndex];

					std::vector<SprEntry>& sprEntries = (packedData & packedDataMask) ? sprSetEntry.SprTexEntries : sprSetEntry.SprEntries;
					sprEntries.emplace_back();
					SprEntry& sprEntry = sprEntries.back();

					sprEntry.ID = id;
					sprEntry.Name = reader.ReadStr(nameOffset);
				}
			});
		}
	}

	SprSetEntry* SprDB::GetSprSetEntry(const std::string & name)
	{
		for (auto& entry : Entries) 
			if (entry.Name == name) 
				return &entry;
		return nullptr;
	}
}