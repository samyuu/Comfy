#include "AetDB.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	using namespace IO;

	AetSceneEntry* AetSetEntry::GetSceneEntry(std::string_view name)
	{
		return FindIfOrNull(SceneEntries, [&](const auto& e) { return e.Name == name; });
	}

	AetSetEntry* AetDB::GetAetSetEntry(std::string_view name)
	{
		return FindIfOrNull(Entries, [&](const auto& e) { return e.Name == name; });
	}

	StreamResult AetDB::Read(StreamReader& reader)
	{
		const auto baseHeader = SectionHeader::TryRead(reader, SectionSignature::AEDB);
		SectionHeader::ScanPOFSectionsSetPointerMode(reader);

		if (baseHeader.has_value())
		{
			reader.SetEndianness(baseHeader->Endianness);
			reader.Seek(baseHeader->StartOfSubSectionAddress());

			if (reader.GetPointerMode() == PtrMode::Mode64Bit)
				reader.PushBaseOffset();
		}

		const auto setEntryCount = reader.ReadU32();
		if (reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.Skip(static_cast<FileAddr>(sizeof(u32)));
		const auto setOffset = reader.ReadPtr();

		const auto sceneCount = reader.ReadU32();
		if (reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.Skip(static_cast<FileAddr>(sizeof(u32)));
		const auto sceneOffset = reader.ReadPtr();

		if (setEntryCount > 0)
		{
			if (!reader.IsValidPointer(setOffset))
				return StreamResult::BadPointer;

			Entries.resize(setEntryCount);
			reader.ReadAtOffsetAware(setOffset, [this](StreamReader& reader)
			{
				for (auto& setEntry : Entries)
				{
					setEntry.ID = AetSetID(reader.ReadU32());
					if (reader.GetPointerMode() == PtrMode::Mode64Bit)
						reader.Skip(static_cast<FileAddr>(sizeof(u32)));

					setEntry.Name = reader.ReadStrPtrOffsetAware();
					setEntry.FileName = reader.ReadStrPtrOffsetAware();
					const auto index = reader.ReadU32();
					setEntry.SprSetID = SprSetID(reader.ReadU32());
				}
			});
		}

		if (sceneCount > 0)
		{
			if (!reader.IsValidPointer(sceneOffset))
				return StreamResult::BadPointer;

			reader.ReadAtOffsetAware(sceneOffset, [this, sceneCount](StreamReader& reader)
			{
				for (u32 i = 0; i < sceneCount; i++)
				{
					const auto id = AetSceneID(reader.ReadU32());
					if (reader.GetPointerMode() == PtrMode::Mode64Bit)
						reader.Skip(static_cast<FileAddr>(sizeof(u32)));

					const auto nameOffset = reader.ReadPtr();
					const auto packedData = reader.ReadU32();

					const auto sceneIndex = static_cast<u16>(packedData & 0xFFFF);
					const auto setIndex = static_cast<u16>((packedData >> 16) & 0xFFFF);

					assert(InBounds(setIndex, Entries));
					auto& setEntry = Entries[setIndex];
					auto& sceneEntry = setEntry.SceneEntries.emplace_back();

					sceneEntry.ID = id;
					sceneEntry.Name = reader.ReadStrAtOffsetAware(nameOffset);
					sceneEntry.Index = sceneIndex;
				}
			});
		}
		
		if (baseHeader.has_value() && reader.GetPointerMode() == PtrMode::Mode64Bit)
			reader.PopBaseOffset();

		return StreamResult::Success;
	}

	StreamResult AetDB::Write(StreamWriter& writer)
	{
		writer.WriteU32(static_cast<u32>(Entries.size()));
		writer.WriteFuncPtr([&](StreamWriter& writer)
		{
			u32 setIndex = 0;
			for (const auto& setEntry : Entries)
			{
				writer.WriteU32(static_cast<u32>(setEntry.ID));
				writer.WriteStrPtr(setEntry.Name);
				writer.WriteStrPtr(setEntry.FileName);
				writer.WriteU32(setIndex++);
				writer.WriteU32(static_cast<u32>(setEntry.SprSetID));
			}
		});

		size_t sceneCount = 0;
		for (const auto& setEntry : Entries)
			sceneCount += setEntry.SceneEntries.size();

		writer.WriteU32(static_cast<u32>(sceneCount));
		writer.WriteFuncPtr([&](StreamWriter& writer)
		{
			u16 setIndex = 0;
			for (const auto& setEntry : Entries)
			{
				u16 sceneIndex = 0;
				for (const auto& sceneEntry : setEntry.SceneEntries)
				{
					writer.WriteU32(static_cast<u32>(sceneEntry.ID));
					writer.WriteStrPtr(sceneEntry.Name);
					writer.WriteU32(static_cast<u32>(setIndex << 16) | static_cast<u32>(sceneEntry.Index));
					sceneIndex++;
				}
				setIndex++;
			}
		});

		writer.WritePadding(16);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);

		return StreamResult::Success;
	}
}
