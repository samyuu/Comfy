#include "TexDB.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	using namespace IO;

	void TexDB::Read(StreamReader& reader)
	{
		const auto texEntryCount = reader.ReadU32();
		const auto texOffset = reader.ReadPtr();

		if (texEntryCount > 0 && texOffset != FileAddr::NullPtr)
		{
			Entries.resize(texEntryCount);
			reader.ReadAtOffsetAware(texOffset, [this](StreamReader& reader)
			{
				for (auto& texEntry : Entries)
				{
					texEntry.ID = TexID(reader.ReadU32());
					texEntry.Name = reader.ReadStrPtrOffsetAware();
				}
			});
		}
	}

	void TexDB::Write(StreamWriter& writer)
	{
		writer.WriteU32(static_cast<u32>(Entries.size()));
		writer.WriteFuncPtr([this](StreamWriter& writer)
		{
			for (auto& texEntry : Entries)
			{
				writer.WriteU32(static_cast<u32>(texEntry.ID));
				writer.WriteStrPtr(texEntry.Name);
			}

			writer.WriteAlignmentPadding(16);
			writer.WritePadding(16);
		});

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);
	}
	
	const TexEntry* TexDB::GetTexEntry(TexID id) const
	{
		return FindIfOrNull(Entries, [&](const auto& e) { return (e.ID == id); });
	}

	const TexEntry* TexDB::GetTexEntry(std::string_view name) const
	{
		return FindIfOrNull(Entries, [&](const auto& e) { return (e.Name == name); });
	}
}
