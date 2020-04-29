#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "FileSystem/Stream/FileStream.h"

namespace Comfy::FileSystem
{
	namespace FArcEncryption
	{
		static constexpr size_t IVSize = 16;
		static constexpr size_t KeySize = 16;
		static constexpr uint32_t DataAlignment = 16;

		// NOTE: project_diva.bin
		static constexpr std::array<uint8_t, KeySize>  ClassicKey = { 'p', 'r', 'o', 'j', 'e', 'c', 't', '_', 'd', 'i', 'v', 'a', '.', 'b', 'i', 'n' };

		// NOTE: 1372D57B6E9E31EBA239B83C1557C6BB
		static constexpr std::array<uint8_t, KeySize> ModernKey = { 0x13, 0x72, 0xD5, 0x7B, 0x6E, 0x9E, 0x31, 0xEB, 0xA2, 0x39, 0xB8, 0x3C, 0x15, 0x57, 0xC6, 0xBB };

		static constexpr std::array<uint8_t, IVSize> DummyIV = { 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC };

		constexpr size_t GetPaddedSize(const size_t dataSize)
		{
			return (dataSize + (DataAlignment - 1)) & ~(DataAlignment - 1);
		}
	}

	enum class FArcSignature : uint32_t
	{
		UnCompressed = 'FArc',
		Compressed = 'FArC',
		Extended = 'FARC',
		Reserved = 'FARc',
	};

	enum FArcFlags : uint32_t
	{
		FArcFlags_None = 0,
		FArcFlags_Reserved = 1 << 0,
		FArcFlags_Compressed = 1 << 1,
		FArcFlags_Encrypted = 1 << 2,
	};

	enum class FArcEncryptionFormat
	{
		None,
		Classic,
		Modern,
	};

	class FArc;

	class FArcEntry
	{
	public:
		FArcEntry(FArc& parent) : parentFArc(parent) {}
		~FArcEntry() = default;

	public:
		std::string Name;
		FileAddr Offset;
		size_t CompressedSize;
		size_t OriginalSize;

		// NOTE: Has to be sufficiently large to store all of OriginalSize
		void ReadIntoBuffer(void* outFileContent) const;
		UniquePtr<uint8_t[]> ReadArray() const;

	private:
		FArc& parentFArc;
	};

	class FArc : NonCopyable
	{
		friend class FArcEntry;

	public:
		static UniquePtr<FArc> Open(std::string_view filePath);

	public:
		FArc() = default;
		~FArc();

	public:
		std::vector<FArcEntry>& GetEntries();
		const FArcEntry* FindFile(std::string_view name);

	protected:
		FileStream stream = {};

		FArcSignature signature = FArcSignature::UnCompressed;
		FArcFlags flags = FArcFlags_None;
		uint32_t alignment = 0;
		bool isModern = false;

		std::vector<FArcEntry> entries;

		FArcEncryptionFormat encryptionFormat = FArcEncryptionFormat::None;
		std::array<uint8_t, FArcEncryption::IVSize> aesIV = FArcEncryption::DummyIV;

	protected:
		bool OpenStream(std::string_view filePath);
		void ReadEntryContent(const FArcEntry& entry, void* outFileContent);

	private:
		bool ParseHeaderAndEntries();
		bool ParseAdvanceSingleEntry(const uint8_t*& headerDataPointer, const uint8_t* const headerEnd);
		bool ParseAllEntriesByRange(const uint8_t* headerData, const uint8_t* headerEnd);
		bool ParseAllEntriesByCount(const uint8_t* headerData, size_t entryCount, const uint8_t* const headerEnd);
		bool DecryptFileContent(const uint8_t* encryptedData, uint8_t* decryptedData, size_t dataSize);
	};
}
