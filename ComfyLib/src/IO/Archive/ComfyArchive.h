#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "IO/Stream/Stream.h"

namespace Comfy::IO
{
	enum class EntryType : u32
	{
		File = 'elif',
		Directory = 'rid',
		Root = 'toor',
		None = 'llun',
	};

	struct ComfyEntryFlags
	{
		// NOTE: Created by the original tool chain
		u32 Verified : 1;
		// TODO: To be determined
		u32 IsEncrypted : 1;
		u32 IsCompressed : 1;
		u32 Reserved : 1;
	};

	struct ComfyEntry
	{
		EntryType Type;
		ComfyEntryFlags Flags;

		const char* Name;
		size_t Size;
		u64 Offset;
	};

	struct ComfyDirectory
	{
		EntryType Type;
		ComfyEntryFlags Flags;

		const char* Name;
		size_t EntryCount;
		ComfyEntry* Entries;

		size_t SubDirectoryCount;
		ComfyDirectory* SubDirectories;
	};

	struct ComfyVersion
	{
		u8 Major;
		u8 Minor;
		u16 Reserved;
	};

	struct ComfyArchiveFlags
	{
		// NOTE: 64-bit pointers and sizes
		u64 WideAddresses : 1;
		// NOTE: Encrypted file and directory names
		u64 EncryptedStrings : 1;
		// NOTE: Created by the original tool chain
		u64 Verified : 1;
	};

	struct ComfyArchiveHeader
	{
		std::array<u8, 4> Magic;

		ComfyVersion Version;

		std::array<u8, 4> CreatorID;
		std::array<u8, 4> ReservedID;

		__time64_t CreationDate;
		ComfyArchiveFlags Flags;

		std::array<u8, 16> IV;

		size_t DataSize;
		u64 DataOffset;
	};

	class ComfyArchive : NonCopyable
	{
	public:
		static constexpr std::array<u8, 4> Magic = { 0xCF, 0x5C, 0xAC, 0x90 };
		static constexpr ComfyVersion Version = { 0x01, 0x00 };

		static constexpr char DirectorySeparator = '/';

	public:
		ComfyArchive() = default;
		~ComfyArchive();

	public:
		void Mount(std::string_view filePath);
		void UnMount();

		const ComfyArchiveHeader& GetHeader() const;
		const ComfyDirectory& GetRootDirectory() const;

		const ComfyEntry* FindFile(std::string_view filePath) const;
		const ComfyEntry* FindFileInDirectory(const ComfyDirectory* directory, std::string_view fileName) const;

		const ComfyDirectory* FindDirectory(std::string_view directoryPath) const;

		bool ReadFileIntoBuffer(std::string_view filePath, std::vector<u8>& buffer);

		bool ReadEntryIntoBuffer(const ComfyEntry* entry, void* outputBuffer);

	private:
		const ComfyDirectory* FindNestedDirectory(const ComfyDirectory* parent, std::string_view directory) const;
		const ComfyDirectory* FindDirectory(const ComfyDirectory* parent, std::string_view directoryName) const;

	private:
		void ParseEntries();
		void LinkRemapPointers();
		void DecryptStrings();

	private:
		bool isMounted = false;

		ComfyArchiveHeader header = {};
		ComfyDirectory* rootDirectory = nullptr;

		UniquePtr<u8[]> dataBuffer = nullptr;
		UniquePtr<IStream> dataStream = nullptr;
	};
}
