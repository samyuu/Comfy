#include "TexDB.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	using namespace IO;

	void TexDB::Read(StreamReader& reader)
	{
		u32 texEntryCount = reader.ReadU32();
		FileAddr texOffset = reader.ReadPtr();

		if (texEntryCount > 0 && texOffset != FileAddr::NullPtr)
		{
			Entries.resize(texEntryCount);
			reader.ReadAt(texOffset, [this](StreamReader& reader)
			{
				for (auto& texEntry : Entries)
				{
					texEntry.ID = TexID(reader.ReadU32());
					texEntry.Name = reader.ReadStrPtr();
				}
			});
		}
	}

	void TexDB::Write(StreamWriter& writer)
	{
		writer.WriteU32(static_cast<u32>(Entries.size()));
		writer.WritePtr([this](StreamWriter& writer)
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
		auto found = std::find_if(Entries.begin(), Entries.end(), [id](auto& e) { return e.ID == id; });
		return (found == Entries.end()) ? nullptr : &(*found);
	}

	const TexEntry* TexDB::GetTexEntry(std::string_view name) const
	{
		auto found = std::find_if(Entries.begin(), Entries.end(), [name](auto& e) { return e.Name == name; });
		return (found == Entries.end()) ? nullptr : &(*found);
	}
}
