#include "SprDB.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	using namespace IO;

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

	void SprDB::Read(StreamReader& reader)
	{
		u32 sprSetEntryCount = reader.ReadU32();
		FileAddr sprSetOffset = reader.ReadPtr();

		u32 sprEntryCount = reader.ReadU32();
		FileAddr sprOffset = reader.ReadPtr();

		if (sprSetEntryCount > 0 && sprSetOffset != FileAddr::NullPtr)
		{
			Entries.resize(sprSetEntryCount);
			reader.ReadAt(sprSetOffset, [this](StreamReader& reader)
			{
				for (auto& sprSetEntry : Entries)
				{
					sprSetEntry.ID = SprSetID(reader.ReadU32());
					sprSetEntry.Name = reader.ReadStrPtr();
					sprSetEntry.FileName = reader.ReadStrPtr();
					u32 index = reader.ReadI32();
				}
			});
		}

		if (sprEntryCount > 0 && sprOffset != FileAddr::NullPtr)
		{
			reader.ReadAt(sprOffset, [this, sprEntryCount](StreamReader& reader)
			{
				for (u32 i = 0; i < sprEntryCount; i++)
				{
					constexpr u16 packedDataMask = 0x1000;

					SprID id = SprID(reader.ReadU32());
					FileAddr nameOffset = reader.ReadPtr();
					i16 index = reader.ReadI16();
					u16 packedData = reader.ReadU16();

					i32 sprSetEntryIndex = (packedData & ~packedDataMask);
					SprSetEntry& sprSetEntry = Entries[sprSetEntryIndex];

					std::vector<SprEntry>& sprEntries = (packedData & packedDataMask) ? sprSetEntry.SprTexEntries : sprSetEntry.SprEntries;
					sprEntries.emplace_back();
					SprEntry& sprEntry = sprEntries.back();

					sprEntry.ID = id;
					sprEntry.Name = reader.ReadStrAt(nameOffset);
					sprEntry.Index = index;
				}
			});
		}
	}

	void SprDB::Write(StreamWriter& writer)
	{
		const auto startPosition = writer.GetPosition();

		writer.WriteU32(GetSprSetEntryCount());
		writer.WritePtr(FileAddr::NullPtr);
		writer.WriteU32(GetSprEntryCount());
		writer.WritePtr(FileAddr::NullPtr);

		writer.SetPosition(startPosition + FileAddr(0xC));
		writer.WritePtr([this](StreamWriter& writer)
		{
			i16 sprSetIndex = 0;
			for (auto& sprSetEntry : Entries)
			{
				constexpr u16 packedDataMask = 0x1000;

				for (auto& sprTexEntry : sprSetEntry.SprTexEntries)
				{
					writer.WriteU32(static_cast<u32>(sprTexEntry.ID));
					writer.WriteStrPtr(sprTexEntry.Name);
					writer.WriteI16(sprTexEntry.Index);
					writer.WriteU16(sprSetIndex | packedDataMask);
				}

				i16 sprIndex = 0;
				for (auto& sprEntry : sprSetEntry.SprEntries)
				{
					writer.WriteU32(static_cast<u32>(sprEntry.ID));
					writer.WriteStrPtr(sprEntry.Name);
					writer.WriteI16(sprEntry.Index);
					writer.WriteU16(sprSetIndex);
				}

				sprSetIndex++;
			}
			writer.WriteAlignmentPadding(16);
			writer.WritePadding(16);
		});

		writer.SetPosition(startPosition + FileAddr(0x4));
		writer.WritePtr([this](StreamWriter& writer)
		{
			i32 index = 0;
			for (auto& sprSetEntry : Entries)
			{
				writer.WriteU32(static_cast<u32>(sprSetEntry.ID));
				writer.WriteStrPtr(sprSetEntry.Name);
				writer.WriteStrPtr(sprSetEntry.FileName);
				writer.WriteI32(index++);
			}
			writer.WriteAlignmentPadding(16);
			writer.WritePadding(16);
		});

		writer.SetPosition(startPosition + FileAddr(0x10));
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

	u32 SprDB::GetSprSetEntryCount()
	{
		return static_cast<u32>(Entries.size());
	}

	u32 SprDB::GetSprEntryCount()
	{
		u32 sprEntryCount = 0;
		for (auto& sprSetEntry : Entries)
			sprEntryCount += static_cast<u32>(sprSetEntry.SprEntries.size() + sprSetEntry.SprTexEntries.size());
		return sprEntryCount;
	}
}
