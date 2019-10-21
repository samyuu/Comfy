#include "Farc.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"
#include <zlib.h>
#include <plusaes.hpp>

namespace FileSystem
{
	static inline uint32_t GetAesDataAlignmentSize(uint64_t dataSize)
	{
		constexpr uint32_t aesAlignmentSize = 16;
		return (static_cast<uint32_t>(dataSize) + (aesAlignmentSize - 1)) & ~(aesAlignmentSize - 1);
	}

	// NOTE: project_diva.bin
	uint8_t Farc::ProjectDivaBinKey[KeySize] = { 'p', 'r', 'o', 'j', 'e', 'c', 't', '_', 'd', 'i', 'v', 'a', '.', 'b', 'i', 'n' };

	// NOTE: 1372D57B6E9E31EBA239B83C1557C6BB
	uint8_t Farc::OrbisFutureToneKey[KeySize] = { 0x13, 0x72, 0xD5, 0x7B, 0x6E, 0x9E, 0x31, 0xEB, 0xA2, 0x39, 0xB8, 0x3C, 0x15, 0x57, 0xC6, 0xBB };

	Farc::Farc()
	{
	}

	Farc::~Farc()
	{
		stream.Close();
	}

	RefPtr<Farc> Farc::Open(const std::string& filePath)
	{
		const std::wstring widePath = Utf8ToUtf16(filePath);

		RefPtr<Farc> farc = MakeRef<Farc>();
		if (!farc->OpenStream(widePath))
		{
			Logger::LogLine(__FUNCTION__"(): Unable to open '%s'", filePath.c_str());
			return nullptr;
		}

		if (!farc->ParseEntries())
		{
			Logger::LogLine(__FUNCTION__"(): Unable to parse '%s'", filePath.c_str());
			return nullptr;
		}

		return farc;
	}

	bool Farc::OpenStream(const std::wstring& filePath)
	{
		stream.OpenRead(filePath);

		if (stream.IsOpen())
		{
			reader.OpenStream(&stream);
			reader.SetEndianness(Endianness::Big);
			return true;
		}

		return false;
	}

	bool Farc::ParseEntries()
	{
		if (reader.GetLength() <= sizeof(uint32_t[2]))
			return false;

		uint32_t farcInfo[2];
		reader.Read(farcInfo, sizeof(farcInfo));

		uint32_t signature = ByteswapUInt32(farcInfo[0]);
		uint32_t headerSize = ByteswapUInt32(farcInfo[1]);

		if (reader.GetLength() <= (reader.GetPosition() + headerSize))
			return false;

		if (signature == FarcSignature_Normal || signature == FarcSignature_Compressed)
		{
			encryptionFormat = FarcEncryptionFormat::None;

			alignment = reader.ReadUInt32();
			flags = (signature == FarcSignature_Compressed) ? FarcFlags_Compressed : FarcFlags_None;

			headerSize -= sizeof(alignment);

			uint8_t* headerData = new uint8_t[headerSize];
			reader.Read(headerData, headerSize);

			uint8_t* currentHeaderPosition = headerData;
			uint8_t* headerEnd = headerData + headerSize;

			ParseEntriesInternal(currentHeaderPosition, headerEnd);

			delete[] headerData;
		}
		else if (signature == FarcSignature_Encrypted)
		{
			uint32_t formatData[2];
			reader.Read(formatData, sizeof(formatData));
			flags = ByteswapUInt32(formatData[0]);
			alignment = ByteswapUInt32(formatData[1]);

			// NOTE: Peek at the next 8 bytes which are either the alignment value followed by padding or the start of the AES IV
			uint32_t nextData[2];
			reader.Read(nextData, sizeof(nextData));
			reader.SetPosition(reader.GetPosition() - sizeof(nextData));

			// NOTE: If the padding is not zero and the potential alignment value is unreasonably high we treat it as an encrypted entry table
			constexpr uint32_t reasonableAlignmentThreshold = 0x1000;
			bool encryptedEntries = (flags & FarcFlags_Encrypted) && (nextData[1] != 0) && (ByteswapUInt32(nextData[0]) >= reasonableAlignmentThreshold);

			if (encryptedEntries)
			{
				encryptionFormat = FarcEncryptionFormat::OrbisFutureTone;
				reader.Read(aesIV, sizeof(aesIV));

				uint32_t paddedHeaderSize = GetAesDataAlignmentSize(headerSize);

				// NOTE: Allocate encrypted and decrypted data as a continuous block
				uint8_t* headerData = new uint8_t[paddedHeaderSize * 2];

				uint8_t* encryptedHeaderData = headerData;
				uint8_t* decryptedHeaderData = headerData + paddedHeaderSize;

				reader.Read(encryptedHeaderData, paddedHeaderSize);

				DecryptFileInternal(encryptedHeaderData, decryptedHeaderData, paddedHeaderSize);

				uint8_t* currentHeaderPosition = decryptedHeaderData;
				uint8_t* headerEnd = decryptedHeaderData + headerSize;

				alignment = ByteswapUInt32(*reinterpret_cast<uint32_t*>(currentHeaderPosition));
				currentHeaderPosition += sizeof(uint32_t);
				currentHeaderPosition += sizeof(uint32_t);

				uint32_t entryCount = ByteswapUInt32(*reinterpret_cast<uint32_t*>(currentHeaderPosition));
				currentHeaderPosition += sizeof(uint32_t);
				currentHeaderPosition += sizeof(uint32_t);

				ParseEntriesInternal(currentHeaderPosition, entryCount);

				delete[] headerData;
			}
			else
			{
				encryptionFormat = FarcEncryptionFormat::ProjectDivaBin;

				headerSize -= sizeof(formatData);

				uint8_t* headerData = new uint8_t[headerSize];
				reader.Read(headerData, headerSize);

				uint8_t* currentHeaderPosition = headerData;
				uint8_t* headerEnd = headerData + headerSize;

				alignment = ByteswapUInt32(*reinterpret_cast<uint32_t*>(currentHeaderPosition));
				currentHeaderPosition += sizeof(uint32_t);

				// Padding (?)
				currentHeaderPosition += sizeof(uint32_t);
				currentHeaderPosition += sizeof(uint32_t);

				ParseEntriesInternal(currentHeaderPosition, headerEnd);

				delete[] headerData;
			}
		}
		else
		{
			// NOTE: Not a farc file or invalid format, might wanna add some error logging
			return false;
		}

		return true;
	}

	void Farc::ReadArchiveEntry(const ArchiveEntry& entry, void* fileContentOut)
	{
		stream.Seek(entry.FileOffset);

		if (flags == FarcFlags_None)
		{
			reader.Read(fileContentOut, entry.FileSize);
		}
		else if (flags & FarcFlags_Compressed)
		{
			// NOTE: Since the farc file size is only stored in a 32bit integer, decompressing it as a single block should be safe enough (?)
			uint32_t paddedSize = GetAesDataAlignmentSize(entry.CompressedSize) + 16;

			uint8_t* encryptedData = nullptr;
			uint8_t* compressedData = new uint8_t[paddedSize];

			if (flags & FarcFlags_Encrypted)
			{
				encryptedData = new uint8_t[paddedSize];
				reader.Read(encryptedData, entry.CompressedSize);

				DecryptFileInternal(encryptedData, compressedData, paddedSize);
			}
			else
			{
				reader.Read(compressedData, entry.CompressedSize);
			}

			// NOTE: Could this be related to the IV size?
			uint32_t dataOffset = (encryptionFormat == FarcEncryptionFormat::OrbisFutureTone) ? 16 : 0;

			z_stream zStream;
			zStream.zalloc = Z_NULL;
			zStream.zfree = Z_NULL;
			zStream.opaque = Z_NULL;
			zStream.avail_in = static_cast<uInt>(entry.CompressedSize);
			zStream.next_in = reinterpret_cast<Bytef*>(compressedData + dataOffset);
			zStream.avail_out = static_cast<uInt>(entry.FileSize);
			zStream.next_out = reinterpret_cast<Bytef*>(fileContentOut);

			int initResult = inflateInit2(&zStream, 31);
			assert(initResult == Z_OK);

			// NOTE: This will sometimes fail with Z_DATA_ERROR "incorrect data check" which I believe to be caused by alignment issues with the very last data block of the file
			// NOTE: The file content should however still have been inflated correctly
			int inflateResult = inflate(&zStream, Z_FINISH);
			// assert(inflateResult == Z_STREAM_END && zStream.msg == nullptr);

			int endResult = inflateEnd(&zStream);
			assert(endResult == Z_OK);

			delete[] encryptedData;
			delete[] compressedData;
		}
		else if (flags & FarcFlags_Encrypted)
		{
			uint32_t paddedSize = GetAesDataAlignmentSize(entry.CompressedSize);
			uint8_t* encryptedData = new uint8_t[paddedSize];

			reader.Read(encryptedData, entry.FileSize);
			uint8_t* fileOutput = reinterpret_cast<uint8_t*>(fileContentOut);

			if (paddedSize == entry.FileSize)
			{
				DecryptFileInternal(encryptedData, fileOutput, paddedSize);
			}
			else
			{
				// NOTE: Suboptimal temporary file copy to avoid AES padding issues. All encrypted farcs should however always be either compressed or have correct alignment
				uint8_t* decryptedData = new uint8_t[paddedSize];
				{
					DecryptFileInternal(encryptedData, decryptedData, paddedSize);
					std::copy(decryptedData, decryptedData + entry.FileSize, fileOutput);
				}
				delete[] decryptedData;
			}

			delete[] encryptedData;
		}
		else
		{
			// NOTE: Invalid FarcFlags
			assert(false);
		}
	}

	bool Farc::ParseEntryInternal(const uint8_t*& headerDataPointer)
	{
		ArchiveEntry entry(this);

		entry.Name = std::string(reinterpret_cast<const char*>(headerDataPointer));
		headerDataPointer += entry.Name.size() + sizeof(char);

		entry.FileOffset = ByteswapUInt32(*reinterpret_cast<const uint32_t*>(headerDataPointer));
		headerDataPointer += sizeof(uint32_t);

		uint32_t fileSize = ByteswapUInt32(*reinterpret_cast<const uint32_t*>(headerDataPointer));
		headerDataPointer += sizeof(uint32_t);

		if (flags & FarcFlags_Compressed)
		{
			entry.CompressedSize = fileSize;
			entry.FileSize = ByteswapUInt32(*reinterpret_cast<const uint32_t*>(headerDataPointer));

			headerDataPointer += sizeof(uint32_t);
		}
		else
		{
			entry.CompressedSize = fileSize;
			entry.FileSize = fileSize;
		}

		if (entry.FileOffset + entry.CompressedSize > static_cast<uint64_t>(reader.GetLength()))
		{
			assert(false);
			false;
		}

		if (encryptionFormat == FarcEncryptionFormat::OrbisFutureTone)
		{
			uint32_t unknown = ByteswapUInt32(*reinterpret_cast<const uint32_t*>(headerDataPointer));
			headerDataPointer += sizeof(uint32_t);
		}

		archiveEntries.push_back(entry);

		return true;
	}

	bool Farc::ParseEntriesInternal(const uint8_t* headerStart, const uint8_t* headerEnd)
	{
		while (headerStart < headerEnd)
		{
			if (!ParseEntryInternal(headerStart))
				break;
		}

		return true;
	}

	bool Farc::ParseEntriesInternal(const uint8_t* headerData, uint32_t entryCount)
	{
		for (uint32_t i = 0; i < entryCount; i++)
		{
			if (!ParseEntryInternal(headerData))
				break;
		}

		return true;
	}

	bool Farc::DecryptFileInternal(const uint8_t* encryptedData, uint8_t* decryptedData, uint32_t dataSize)
	{
		if (encryptionFormat == FarcEncryptionFormat::ProjectDivaBin)
		{
			auto result = plusaes::decrypt_ecb(encryptedData, dataSize, ProjectDivaBinKey, KeySize, decryptedData, dataSize, nullptr);
			assert(result == plusaes::Error::kErrorOk);
		}
		else if (encryptionFormat == FarcEncryptionFormat::OrbisFutureTone)
		{
			auto result = plusaes::decrypt_cbc(encryptedData, dataSize, OrbisFutureToneKey, KeySize, &aesIV, decryptedData, dataSize, nullptr);
			assert(result == plusaes::Error::kErrorOk);
		}
		else
		{
			assert(false);
			return false;
		}

		return true;
	}
}