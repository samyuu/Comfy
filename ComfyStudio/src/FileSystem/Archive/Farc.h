#pragma once
#include "FileArchive.h"
#include "FileSystem/Stream/FileStream.h"
#include "FileSystem/BinaryReader.h"

namespace FileSystem
{
	enum FarcSignature : uint32_t
	{
		FarcSignature_Normal = 'FArc',
		FarcSignature_Compressed = 'FArC',
		FarcSignature_Encrypted = 'FARC',
	};

	enum FarcFlags : uint32_t
	{
		FarcFlags_None = 0,
		FarcFlags_Compressed = 1 << 1,
		FarcFlags_Encrypted = 1 << 2,
	};

	enum class FarcEncryptionFormat
	{
		None, 
		ProjectDivaBin, 
		OrbisFutureTone
	};

	class Farc : public FileArchive
	{
	public:
		static constexpr size_t IVSize = 16;
		static constexpr size_t KeySize = 16;

		static uint8_t ProjectDivaBinKey[KeySize];
		static uint8_t OrbisFutureToneKey[KeySize];

	public:
		Farc();
		~Farc();

		static RefPtr<Farc> Open(const String& filePath);

	protected:
		FileStream stream;
		BinaryReader reader;
		uint32_t alignment;
		uint32_t flags;
		FarcEncryptionFormat encryptionFormat;
		uint8_t aesIV[IVSize];

	protected:
		bool OpenStream(const WideString& filePath);
		bool ParseEntries();

		virtual void ReadArchiveEntry(const ArchiveEntry& entry, void* fileContentOut) override;

	private:
		bool ParseEntryInternal(const uint8_t*& headerDataPointer);
		bool ParseEntriesInternal(const uint8_t* headerData, const uint8_t* headerEnd);
		bool ParseEntriesInternal(const uint8_t* headerData, uint32_t entryCount);
		bool DecryptFileInternal(const uint8_t* encryptedData, uint8_t* decryptedData, uint32_t dataSize);
	};
}