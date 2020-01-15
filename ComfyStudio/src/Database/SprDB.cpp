#include "SprDB.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"

namespace Database
{
	using namespace FileSystem;

	SprEntry* SprSetEntry::GetSprEntry(SprID id)
	{
		for (auto& entry : SprEntries)
			if (entry.ID == id)
				return &entry;
		return nullptr;
	}

	SprEntry* SprSetEntry::GetSprEntry(std::string_view name)
	{
		for (auto& entry : SprEntries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}

	SprEntry* SprSetEntry::GetSprTexEntry(std::string_view name)
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
					sprSetEntry.ID = SprSetID(reader.ReadUInt32());
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

					SprID id = SprID(reader.ReadUInt32());
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
					sprEntry.Index = index;
				}
			});
		}
	}

	void SprDB::Write(BinaryWriter& writer)
	{
		int64_t startPosition = writer.GetPosition();

		writer.WriteUInt32(GetSprSetEntryCount());
		writer.WritePtr(nullptr);
		writer.WriteUInt32(GetSprEntryCount());
		writer.WritePtr(nullptr);

		writer.SetPosition(startPosition + 0xC);
		writer.WritePtr([this](BinaryWriter& writer)
		{
			int16_t sprSetIndex = 0;
			for (auto& sprSetEntry : Entries)
			{
				constexpr uint16_t packedDataMask = 0x1000;

				for (auto& sprTexEntry : sprSetEntry.SprTexEntries)
				{
					writer.WriteUInt32(static_cast<uint32_t>(sprTexEntry.ID));
					writer.WriteStrPtr(&sprTexEntry.Name);
					writer.WriteInt16(sprTexEntry.Index);
					writer.WriteUInt16(sprSetIndex | packedDataMask);
				}

				int16_t sprIndex = 0;
				for (auto& sprEntry : sprSetEntry.SprEntries)
				{
					writer.WriteUInt32(static_cast<uint32_t>(sprEntry.ID));
					writer.WriteStrPtr(&sprEntry.Name);
					writer.WriteInt16(sprEntry.Index);
					writer.WriteUInt16(sprSetIndex);
				}

				sprSetIndex++;
			}
			writer.WriteAlignmentPadding(16);
			writer.WritePadding(16);
		});

		writer.SetPosition(startPosition + 0x4);
		writer.WritePtr([this](BinaryWriter& writer)
		{
			int32_t index = 0;
			for (auto& sprSetEntry : Entries)
			{
				writer.WriteUInt32(static_cast<uint32_t>(sprSetEntry.ID));
				writer.WriteStrPtr(&sprSetEntry.Name);
				writer.WriteStrPtr(&sprSetEntry.FileName);
				writer.WriteInt32(index++);
			}
			writer.WriteAlignmentPadding(16);
			writer.WritePadding(16);
		});

		writer.SetPosition(startPosition + 0x10);
		writer.WritePadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);
	}

	SprSetEntry* SprDB::GetSprSetEntry(std::string_view name)
	{
		for (auto& entry : Entries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}

	uint32_t SprDB::GetSprSetEntryCount()
	{
		return static_cast<uint32_t>(Entries.size());
	}

	uint32_t SprDB::GetSprEntryCount()
	{
		uint32_t sprEntryCount = 0;
		for (auto& sprSetEntry : Entries)
			sprEntryCount += static_cast<uint32_t>(sprSetEntry.SprEntries.size() + sprSetEntry.SprTexEntries.size());
		return sprEntryCount;
	}
}
