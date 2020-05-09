#include "FArc.h"
#include "Core/Logger.h"
#include "Misc/EndianHelper.h"
#include "IO/Crypto/Crypto.h"
#include <zlib.h>

namespace Comfy::IO
{
	void FArcEntry::ReadIntoBuffer(void* outFileContent) const
	{
		parentFArc.ReadEntryContent(*this, outFileContent);
	}

	std::unique_ptr<u8[]> FArcEntry::ReadArray() const
	{
		auto result = std::make_unique<u8[]>(OriginalSize);
		ReadIntoBuffer(result.get());
		return result;
	}

	std::unique_ptr<FArc> FArc::Open(std::string_view filePath)
	{
		auto farc = std::make_unique<FArc>();
		if (!farc->OpenStream(filePath))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unable to open '%s'", filePath.data());
			return nullptr;
		}

		if (!farc->ParseHeaderAndEntries())
		{
			Logger::LogErrorLine(__FUNCTION__"(): Unable to parse '%s'", filePath.data());
			return nullptr;
		}

		return farc;
	}

	FArc::~FArc()
	{
		stream.Close();
	}

	std::vector<FArcEntry>& FArc::GetEntries()
	{
		return entries;
	}

	const FArcEntry* FArc::FindFile(std::string_view name)
	{
		const auto foundEntry = std::find_if(entries.begin(), entries.end(), [&](const auto& entry)
		{
			// NOTE: Should this be case insensitive? Maybe...
			return entry.Name == name;
		});

		return (foundEntry != entries.end()) ? &(*foundEntry) : nullptr;
	}

	bool FArc::OpenStream(std::string_view filePath)
	{
		stream.OpenRead(filePath);
		return stream.IsOpen();
	}

	void FArc::ReadEntryContent(const FArcEntry& entry, void* outFileContent)
	{
		if (outFileContent == nullptr || !stream.IsOpen())
			return;

		// NOTE: Could this be related to the IV size?
		const size_t dataOffset = (encryptionFormat == FArcEncryptionFormat::Modern) ? 16 : 0;

		if (flags & FArcFlags_Compressed)
		{
			stream.Seek(entry.Offset);

			// NOTE: Since the farc file size is only stored in a 32bit integer, decompressing it as a single block should be safe enough (?)
			const auto paddedSize = std::min(FArcEncryption::GetPaddedSize(entry.CompressedSize, alignment) + 16, static_cast<size_t>(stream.GetLength()));

			std::unique_ptr<u8[]> encryptedData = nullptr;
			std::unique_ptr<u8[]> compressedData = std::make_unique<u8[]>(paddedSize);

			if (flags & FArcFlags_Encrypted)
			{
				encryptedData = std::make_unique<u8[]>(paddedSize);
				stream.ReadBuffer(encryptedData.get(), paddedSize);

				DecryptFileContent(encryptedData.get(), compressedData.get(), paddedSize);
			}
			else
			{
				stream.ReadBuffer(compressedData.get(), paddedSize);
			}

			z_stream zStream;
			zStream.zalloc = Z_NULL;
			zStream.zfree = Z_NULL;
			zStream.opaque = Z_NULL;
			zStream.avail_in = static_cast<uInt>(paddedSize);
			zStream.next_in = reinterpret_cast<Bytef*>(compressedData.get() + dataOffset);
			zStream.avail_out = static_cast<uInt>(entry.OriginalSize);
			zStream.next_out = reinterpret_cast<Bytef*>(outFileContent);

			const int initResult = inflateInit2(&zStream, 31);
			assert(initResult == Z_OK);

			// NOTE: This will sometimes fail with Z_DATA_ERROR "incorrect data check" which I believe to be caused by alignment issues with the very last data block of the file
			//		 The file content should however still have been inflated correctly
			const int inflateResult = inflate(&zStream, Z_FINISH);
			// assert(inflateResult == Z_STREAM_END && zStream.msg == nullptr);

			const int endResult = inflateEnd(&zStream);
			assert(endResult == Z_OK);
		}
		else if (flags & FArcFlags_Encrypted)
		{
			stream.Seek(entry.Offset);

			const auto paddedSize = FArcEncryption::GetPaddedSize(entry.OriginalSize) + dataOffset;
			auto encryptedData = std::make_unique<u8[]>(paddedSize);

			stream.ReadBuffer(encryptedData.get(), paddedSize);
			u8* fileOutput = reinterpret_cast<u8*>(outFileContent);

			if (paddedSize == entry.OriginalSize)
			{
				DecryptFileContent(encryptedData.get(), fileOutput, paddedSize);
			}
			else
			{
				// NOTE: Suboptimal temporary file copy to avoid AES padding issues. All encrypted farcs should however always be either compressed or have correct alignment
				auto decryptedData = std::make_unique<u8[]>(paddedSize);

				DecryptFileContent(encryptedData.get(), decryptedData.get(), paddedSize);

				const u8* decryptedOffsetData = decryptedData.get() + dataOffset;
				std::copy(decryptedOffsetData, decryptedOffsetData + entry.OriginalSize, fileOutput);
			}
		}
		else
		{
			stream.Seek(entry.Offset);
			stream.ReadBuffer(outFileContent, entry.OriginalSize);
		}
	}

	bool FArc::ParseHeaderAndEntries()
	{
		if (stream.GetLength() <= FileAddr(sizeof(u32[2])))
			return false;

		std::array<u32, 2> parsedSignatureData;
		stream.ReadBuffer(parsedSignatureData.data(), sizeof(parsedSignatureData));

		signature = static_cast<FArcSignature>(Utilities::ByteSwapU32(parsedSignatureData[0]));
		const auto parsedHeaderSize = Utilities::ByteSwapU32(parsedSignatureData[1]);

		// NOTE: Empty but valid FArc
		if (signature == FArcSignature::UnCompressed && parsedHeaderSize <= sizeof(u32))
			return true;

		if (stream.GetLength() <= (stream.GetPosition() + static_cast<FileAddr>(parsedHeaderSize)))
			return false;

		if (signature == FArcSignature::UnCompressed || signature == FArcSignature::Compressed)
		{
			encryptionFormat = FArcEncryptionFormat::None;

			u32 parsedAlignment;
			stream.ReadBuffer(&parsedAlignment, sizeof(parsedAlignment));

			alignment = Utilities::ByteSwapU32(parsedAlignment);
			flags = (signature == FArcSignature::Compressed) ? FArcFlags_Compressed : FArcFlags_None;

			const auto headerSize = (parsedHeaderSize - sizeof(alignment));

			auto headerData = std::make_unique<u8[]>(headerSize);
			stream.ReadBuffer(headerData.get(), headerSize);

			u8* currentHeaderPosition = headerData.get();
			const u8* headerEnd = headerData.get() + headerSize;

			ParseAllEntriesByRange(currentHeaderPosition, headerEnd);
		}
		else if (signature == FArcSignature::Extended)
		{
			std::array<u32, 2> parsedFormatData;
			stream.ReadBuffer(parsedFormatData.data(), sizeof(parsedFormatData));

			flags = static_cast<FArcFlags>(Utilities::ByteSwapU32(parsedFormatData[0]));

			// NOTE: Peek at the next 8 bytes which are either the alignment value followed by padding or the start of the AES IV
			std::array<u32, 2> parsedNextData;
			stream.ReadBuffer(parsedNextData.data(), sizeof(parsedNextData));

			alignment = Utilities::ByteSwapU32(parsedNextData[0]);
			isModern = (parsedNextData[1] != 0);

			// NOTE: If the padding is not zero and the potential alignment value is unreasonably high we treat it as an encrypted entry table
			constexpr u32 reasonableAlignmentThreshold = 0x1000;
			const bool encryptedEntries = (flags & FArcFlags_Encrypted) && isModern && (alignment >= reasonableAlignmentThreshold);

			encryptionFormat = (flags & FArcFlags_Encrypted) ? (encryptedEntries ? FArcEncryptionFormat::Modern : FArcEncryptionFormat::Classic) : FArcEncryptionFormat::None;

			if (encryptedEntries)
			{
				stream.Seek(stream.GetPosition() - FileAddr(sizeof(parsedNextData)));
				stream.ReadBuffer(aesIV.data(), aesIV.size());

				const auto paddedHeaderSize = FArcEncryption::GetPaddedSize(parsedHeaderSize);

				// NOTE: Allocate encrypted and decrypted data as a continuous block
				auto headerData = std::make_unique<u8[]>(paddedHeaderSize * 2);

				u8* encryptedHeaderData = headerData.get();
				u8* decryptedHeaderData = headerData.get() + paddedHeaderSize;

				stream.ReadBuffer(encryptedHeaderData, paddedHeaderSize);

				DecryptFileContent(encryptedHeaderData, decryptedHeaderData, paddedHeaderSize);

				u8* currentHeaderPosition = decryptedHeaderData;
				const u8* headerEnd = decryptedHeaderData + parsedHeaderSize;

				// NOTE: Example data: 00 00 00 10 | 00 00 00 01 | 00 00 00 01 | 00 00 00 10
				alignment = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
				currentHeaderPosition += sizeof(u32);
				currentHeaderPosition += sizeof(u32);

				const u32 entryCount = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
				currentHeaderPosition += sizeof(u32);
				currentHeaderPosition += sizeof(u32);

				ParseAllEntriesByCount(currentHeaderPosition, entryCount, headerEnd);
				assert(entries.size() == entryCount);
			}
			else
			{
				stream.Seek(stream.GetPosition() - FileAddr(sizeof(u32)));

				const auto headerSize = (parsedHeaderSize - 12);

				auto headerData = std::make_unique<u8[]>(headerSize);
				stream.ReadBuffer(headerData.get(), headerSize);

				u8* currentHeaderPosition = headerData.get();
				const u8* headerEnd = currentHeaderPosition + headerSize;

				if (isModern)
				{
					// NOTE: Example data: 00 00 00 01 | 00 00 00 01 | 00 00 00 10
					const u32 reserved = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
					currentHeaderPosition += sizeof(u32);

					const u32 entryCount = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
					currentHeaderPosition += sizeof(u32);

					alignment = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
					currentHeaderPosition += sizeof(u32);

					ParseAllEntriesByCount(currentHeaderPosition, entryCount, headerEnd);
					assert(entries.size() == entryCount);
				}
				else
				{
					const u32 reserved0 = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
					currentHeaderPosition += sizeof(u32);

					const u32 reserved1 = Utilities::ByteSwapU32(*reinterpret_cast<u32*>(currentHeaderPosition));
					currentHeaderPosition += sizeof(u32);

					ParseAllEntriesByRange(currentHeaderPosition, headerEnd);
				}
			}
		}
		else
		{
			// NOTE: Not a FArc file or invalid format, might wanna add some error logging
			return false;
		}

		return true;
	}

	bool FArc::ParseAdvanceSingleEntry(const u8*& headerDataPointer, const u8* const headerEnd)
	{
		auto newEntry = FArcEntry(*this);

		newEntry.Name = std::string(reinterpret_cast<const char*>(headerDataPointer));
		assert(!newEntry.Name.empty());

		headerDataPointer += newEntry.Name.size() + sizeof(char);
		assert(headerDataPointer <= headerEnd);

		newEntry.Offset = static_cast<FileAddr>(Utilities::ByteSwapU32(*reinterpret_cast<const u32*>(headerDataPointer)));

		headerDataPointer += sizeof(u32);
		assert(headerDataPointer <= headerEnd);

		newEntry.CompressedSize = Utilities::ByteSwapU32(*reinterpret_cast<const u32*>(headerDataPointer));

		headerDataPointer += sizeof(u32);
		assert(headerDataPointer <= headerEnd);

		if (signature == FArcSignature::UnCompressed)
		{
			newEntry.OriginalSize = newEntry.CompressedSize;
		}
		else
		{
			newEntry.OriginalSize = Utilities::ByteSwapU32(*reinterpret_cast<const u32*>(headerDataPointer));

			headerDataPointer += sizeof(u32);
			assert(headerDataPointer <= headerEnd);
		}

		if (newEntry.Offset + static_cast<FileAddr>(newEntry.CompressedSize) > stream.GetLength())
		{
			assert(false);
			false;
		}

		if (isModern)
		{
			const u32 parsedReserved = Utilities::ByteSwapU32(*reinterpret_cast<const u32*>(headerDataPointer));
			headerDataPointer += sizeof(u32);
			assert(headerDataPointer <= headerEnd);
		}

		entries.push_back(std::move(newEntry));
		return true;
	}

	bool FArc::ParseAllEntriesByRange(const u8* headerStart, const u8* headerEnd)
	{
		while (headerStart < headerEnd)
		{
			if (!ParseAdvanceSingleEntry(headerStart, headerEnd))
				break;
		}

		return true;
	}

	bool FArc::ParseAllEntriesByCount(const u8* headerData, size_t entryCount, const u8* const headerEnd)
	{
		entries.reserve(entryCount);
		for (size_t i = 0; i < entryCount; i++)
		{
			if (!ParseAdvanceSingleEntry(headerData, headerEnd))
				break;
		}

		return true;
	}

	bool FArc::DecryptFileContent(const u8* encryptedData, u8* decryptedData, size_t dataSize)
	{
		if (encryptionFormat == FArcEncryptionFormat::Classic)
		{
			return Crypto::DecryptAesEcb(encryptedData, decryptedData, dataSize, FArcEncryption::ClassicKey);
		}
		else if (encryptionFormat == FArcEncryptionFormat::Modern)
		{
			return Crypto::DecryptAesCbc(encryptedData, decryptedData, dataSize, FArcEncryption::ModernKey, aesIV);
		}
		else
		{
			assert(false);
		}

		return false;
	}
}
