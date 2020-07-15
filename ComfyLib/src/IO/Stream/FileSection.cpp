#include "FileSection.h"
#include "Manipulator/StreamReader.h"

namespace Comfy::IO
{
	SectionHeader SectionHeader::Read(StreamReader& reader)
	{
		const auto headerAddress = reader.GetPosition();
		const auto signature = static_cast<SectionSignature>(reader.ReadU32_LE());
		const auto sectionSize = reader.ReadU32_LE();
		const auto dataOffset = reader.ReadU32_LE();
		const auto endianness = static_cast<SectionEndianness>(reader.ReadU32_LE());
		const auto depth = reader.ReadU32_LE();
		const auto dataSize = reader.ReadU32_LE();
		const auto reserved0 = reader.ReadU32_LE();
		const auto reserved1 = reader.ReadU32_LE();

		SectionHeader header;
		header.HeaderAddress = headerAddress;
		header.DataOffset = static_cast<FileAddr>(dataOffset);
		header.SectionSize = static_cast<size_t>(sectionSize);
		header.DataSize = static_cast<size_t>(dataSize);
		header.Signature = signature;
		header.Depth = depth;
		header.Endianness = (endianness == SectionEndianness::Big) ? Endianness::Big : Endianness::Little;
		return header;
	}

	std::optional<SectionHeader> SectionHeader::TryRead(StreamReader& reader, SectionSignature expectedSignature)
	{
		const auto startPosition = reader.GetPosition();
		const auto readSignature = static_cast<SectionSignature>(reader.ReadU32_LE());
		reader.Seek(startPosition);

		if (readSignature != expectedSignature)
			return {};

		reader.SetHasSections(true);
		return SectionHeader::Read(reader);
	}

	void SectionHeader::ScanPOFSectionsSetPointerMode(StreamReader& reader)
	{
		if (!reader.GetHasSections() || reader.GetHasBeenPointerModeScanned())
			return;

		reader.ReadAt(FileAddr::NullPtr, [](StreamReader& reader)
		{
			const auto endOfSectionHeaders = reader.GetLength() - FileAddr(32);
			while (reader.GetPosition() < endOfSectionHeaders)
			{
				const auto sectionHeader = SectionHeader::Read(reader);

				if (sectionHeader.Signature == SectionSignature::POF0)
				{
					reader.SetPointerMode(PtrMode::Mode32Bit);
					return;
				}

				if (sectionHeader.Signature == SectionSignature::POF1)
				{
					reader.SetPointerMode(PtrMode::Mode64Bit);
					return;
				}

				const auto endOfSection = sectionHeader.EndOfSubSectionAddress();
				if (endOfSection < reader.GetPosition())
					return;

				reader.Seek(endOfSection);
			}

			// NOTE: In case no relocation tables are found 32-bit should be the default
			reader.SetPointerMode(PtrMode::Mode32Bit);
		});

		reader.SetHasBeenPointerModeScanned(true);
	}
}
