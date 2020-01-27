#include "TxpDB.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/BinaryWriter.h"

namespace Database
{
	using namespace FileSystem;

	void TxpDB::Read(BinaryReader& reader)
	{
		uint32_t txpEntryCount = reader.ReadUInt32();
		void* txpOffset = reader.ReadPtr();

		if (txpEntryCount > 0 && txpOffset != nullptr)
		{
			Entries.resize(txpEntryCount);
			reader.ReadAt(txpOffset, [this](BinaryReader& reader)
			{
				for (auto& txpEntry : Entries)
				{
					txpEntry.ID = TxpID(reader.ReadUInt32());
					txpEntry.Name = reader.ReadStrPtr();
				}
			});
		}
	}

	void TxpDB::Write(BinaryWriter& writer)
	{
		writer.WriteUInt32(static_cast<uint32_t>(Entries.size()));
		writer.WritePtr([this](BinaryWriter& writer)
		{
			for (auto& txpEntry : Entries)
			{
				writer.WriteUInt32(static_cast<uint32_t>(txpEntry.ID));
				writer.WriteStrPtr(txpEntry.Name);
			}

			writer.WriteAlignmentPadding(16);
			writer.WritePadding(16);
		});

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.FlushStringPointerPool();
		writer.WriteAlignmentPadding(16);
	}
	
	const TxpEntry* TxpDB::GetTxpEntry(TxpID id) const
	{
		auto found = std::find_if(Entries.begin(), Entries.end(), [id](auto& e) { return e.ID == id; });
		return (found == Entries.end()) ? nullptr : &(*found);
	}

	const TxpEntry* TxpDB::GetTxpEntry(std::string_view name) const
	{
		auto found = std::find_if(Entries.begin(), Entries.end(), [name](auto& e) { return e.Name == name; });
		return (found == Entries.end()) ? nullptr : &(*found);
	}
}
