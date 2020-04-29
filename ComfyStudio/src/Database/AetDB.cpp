#include "AetDB.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	using namespace IO;

	AetSceneEntry* AetSetEntry::GetSceneEntry(std::string_view name)
	{
		for (auto& entry : SceneEntries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}

	void AetDB::Read(StreamReader& reader)
	{
		u32 setEntryCount = reader.ReadU32();
		FileAddr setOffset = reader.ReadPtr();

		u32 sceneCount = reader.ReadU32();
		FileAddr sceneOffset = reader.ReadPtr();

		if (setEntryCount > 0 && setOffset != FileAddr::NullPtr)
		{
			Entries.resize(setEntryCount);
			reader.ReadAt(setOffset, [this](StreamReader& reader)
			{
				for (auto& setEntry : Entries)
				{
					setEntry.ID = AetSetID(reader.ReadU32());
					setEntry.Name = reader.ReadStrPtr();
					setEntry.FileName = reader.ReadStrPtr();
					u32 index = reader.ReadU32();
					setEntry.SprSetID = SprSetID(reader.ReadU32());
				}
			});
		}

		if (sceneCount > 0 && sceneOffset != FileAddr::NullPtr)
		{
			reader.ReadAt(sceneOffset, [this, sceneCount](StreamReader& reader)
			{
				for (u32 i = 0; i < sceneCount; i++)
				{
					AetSceneID id = AetSceneID(reader.ReadU32());
					FileAddr nameOffset = reader.ReadPtr();
					u16 sceneIndex = reader.ReadU16();
					u16 setIndex = reader.ReadU16();

					AetSetEntry& setEntry = Entries[setIndex];

					setEntry.SceneEntries.emplace_back();
					AetSceneEntry& sceneEntry = setEntry.SceneEntries.back();

					sceneEntry.ID = id;
					sceneEntry.Name = reader.ReadStrAt(nameOffset);
				}
			});
		}
	}

	void AetDB::Write(StreamWriter& writer)
	{
		writer.WriteU32(static_cast<u32>(Entries.size()));
		writer.WritePtr([&](StreamWriter& writer)
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
		writer.WritePtr([&](StreamWriter& writer)
		{
			u16 setIndex = 0;
			for (const auto& setEntry : Entries)
			{
				u16 sceneIndex = 0;
				for (const auto& sceneEntry : setEntry.SceneEntries)
				{
					writer.WriteU32(static_cast<u32>(sceneEntry.ID));
					writer.WriteStrPtr(sceneEntry.Name);
					writer.WriteU16(sceneIndex);
					writer.WriteU16(setIndex);
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
	}

	AetSetEntry* AetDB::GetAetSetEntry(std::string_view name)
	{
		for (auto& entry : Entries)
			if (entry.Name == name)
				return &entry;
		return nullptr;
	}
}
