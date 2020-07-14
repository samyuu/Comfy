#include "SprDB.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	using namespace IO;

	SprEntry* SprSetEntry::GetSprEntry(SprID id)
	{
		return FindIfOrNull(SprEntries, [&](const auto& e) { return (e.ID == id); });
	}

	SprEntry* SprSetEntry::GetSprEntry(std::string_view name)
	{
		return FindIfOrNull(SprEntries, [&](const auto& e) { return (e.Name == name); });
	}

	SprEntry* SprSetEntry::GetSprTexEntry(std::string_view name)
	{
		return FindIfOrNull(SprTexEntries, [&](const auto& e) { return (e.Name == name); });
	}

	void SprDB::Read(StreamReader& reader)
	{
		const auto sprSetEntryCount = reader.ReadU32();
		const auto sprSetOffset = reader.ReadPtr();

		const auto sprEntryCount = reader.ReadU32();
		const auto sprOffset = reader.ReadPtr();

		if (sprSetEntryCount > 0 && sprSetOffset != FileAddr::NullPtr)
		{
			Entries.resize(sprSetEntryCount);
			reader.ReadAtOffsetAware(sprSetOffset, [this](StreamReader& reader)
			{
				for (auto& sprSetEntry : Entries)
				{
					sprSetEntry.ID = SprSetID(reader.ReadU32());
					sprSetEntry.Name = reader.ReadStrPtrOffsetAware();
					sprSetEntry.FileName = reader.ReadStrPtrOffsetAware();
					const auto index = reader.ReadI32();
				}
			});
		}

		if (sprEntryCount > 0 && sprOffset != FileAddr::NullPtr)
		{
			reader.ReadAtOffsetAware(sprOffset, [this, sprEntryCount](StreamReader& reader)
			{
				for (u32 i = 0; i < sprEntryCount; i++)
				{
					constexpr u16 packedDataMask = 0x1000;

					const auto id = SprID(reader.ReadU32());
					const auto nameOffset = reader.ReadPtr();
					const auto index = reader.ReadI16();
					const auto packedData = reader.ReadU16();

					const auto sprSetEntryIndex = (packedData & ~packedDataMask);
					auto& sprSetEntry = Entries[sprSetEntryIndex];

					auto& sprEntries = (packedData & packedDataMask) ? sprSetEntry.SprTexEntries : sprSetEntry.SprEntries;
					auto& sprEntry = sprEntries.emplace_back();

					sprEntry.ID = id;
					sprEntry.Name = reader.ReadStrAtOffsetAware(nameOffset);
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

		writer.Seek(startPosition + FileAddr(0xC));
		writer.WriteFuncPtr([this](StreamWriter& writer)
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

		writer.Seek(startPosition + FileAddr(0x4));
		writer.WriteFuncPtr([this](StreamWriter& writer)
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

		writer.Seek(startPosition + FileAddr(0x10));
		writer.WritePadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);
	}

	SprSetEntry* SprDB::GetSprSetEntry(std::string_view name)
	{
		return FindIfOrNull(Entries, [&](const auto& e) { return (e.Name == name); });
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
