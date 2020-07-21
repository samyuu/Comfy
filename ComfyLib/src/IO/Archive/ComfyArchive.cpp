#include "ComfyArchive.h"
#include "IO/Stream/FileStream.h"
#include "Misc/StringUtil.h"

namespace Comfy::IO
{
	ComfyArchive::~ComfyArchive()
	{
		UnMount();
	}

	bool ComfyArchive::Mount(std::string_view filePath)
	{
		if (isMounted)
			return true;

		isMounted = true;
		dataStream.OpenRead(filePath);

		if (!dataStream.IsOpen())
			return false;

		ParseEntries();
		LinkRemapPointers();

		if (header.Flags.EncryptedStrings)
			DecryptStrings();

		return true;
	}

	void ComfyArchive::UnMount()
	{
		if (!isMounted)
			return;

		if (dataStream.IsOpen())
			dataStream.Close();

		dataBuffer = nullptr;
	}

	const ComfyArchiveHeader& ComfyArchive::GetHeader() const
	{
		assert(isMounted);
		return header;
	}

	const ComfyDirectory& ComfyArchive::GetRootDirectory() const
	{
		assert(isMounted && rootDirectory != nullptr);
		return *rootDirectory;
	}

	const ComfyEntry* ComfyArchive::FindFile(std::string_view filePath) const
	{
		if (rootDirectory == nullptr || filePath.size() < 1)
			return nullptr;

		size_t lastSeparatorIndex = filePath.size();
		for (i64 i = static_cast<i64>(filePath.size()) - 1; i >= 0; i--)
		{
			if (filePath[i] == DirectorySeparator)
			{
				lastSeparatorIndex = i;
				break;
			}
		}

		if (lastSeparatorIndex == filePath.size())
			return FindFileInDirectory(*rootDirectory, filePath);

		std::string_view directory = filePath.substr(0, lastSeparatorIndex);
		const ComfyDirectory* parentDirectory = FindNestedDirectory(*rootDirectory, directory);

		if (parentDirectory == nullptr)
			return nullptr;

		lastSeparatorIndex++;
		std::string_view fileName = filePath.substr(lastSeparatorIndex, filePath.size() - lastSeparatorIndex);

		for (size_t i = 0; i < parentDirectory->EntryCount; i++)
		{
			auto& entry = parentDirectory->Entries[i];

			if (entry.Name == fileName)
				return &entry;
		}

		return nullptr;
	}

	const ComfyEntry* ComfyArchive::FindFileInDirectory(const ComfyDirectory& directory, std::string_view fileName) const
	{
		for (size_t i = 0; i < directory.EntryCount; i++)
		{
			auto& entry = directory.Entries[i];

			if (entry.Name == fileName)
				return &entry;
		}

		return nullptr;
	}

	const ComfyDirectory* ComfyArchive::FindDirectory(std::string_view directoryPath) const
	{
		if (rootDirectory == nullptr)
			return nullptr;

		return FindNestedDirectory(*rootDirectory, directoryPath);
	}

	bool ComfyArchive::ReadFileIntoBuffer(const ComfyEntry* entry, void* outputBuffer)
	{
		if (entry == nullptr)
			return false;

		if (!isMounted || !dataStream.IsOpen() || !dataStream.CanRead())
		{
			assert(false);
			return false;
		}

		dataStream.Seek(static_cast<FileAddr>(entry->Offset));
		dataStream.ReadBuffer(outputBuffer, entry->Size);
		return true;
	}

	const ComfyDirectory* ComfyArchive::FindNestedDirectory(const ComfyDirectory& parent, std::string_view directory) const
	{
		for (size_t i = 0; i < directory.size(); i++)
		{
			if (directory[i] == DirectorySeparator)
			{
				std::string_view parentDirectory = directory.substr(0, i);
				std::string_view subDirectory = directory.substr(i + 1);

				auto result = FindDirectory(parent, parentDirectory);
				if (result == nullptr)
					return nullptr;

				return FindNestedDirectory(*result, subDirectory);
			}
		}

		return FindDirectory(parent, directory);
	}

	const ComfyDirectory* ComfyArchive::FindDirectory(const ComfyDirectory& parent, std::string_view directoryName) const
	{
		for (size_t i = 0; i < parent.SubDirectoryCount; i++)
		{
			auto& subDirectory = parent.SubDirectories[i];

			if (subDirectory.Name == directoryName)
				return &subDirectory;
		}

		return nullptr;
	}

	void ComfyArchive::ParseEntries()
	{
		dataStream.ReadBuffer(&header, sizeof(header));

		// TODO: Extensive validation checking
		assert(header.Magic == ComfyArchive::Magic);

		dataBuffer = std::make_unique<u8[]>(header.DataSize);

		dataStream.Seek(static_cast<FileAddr>(header.DataOffset));
		dataStream.ReadBuffer(dataBuffer.get(), header.DataSize);
	}

	namespace
	{
		template <typename T>
		void RemapFileSpacePointer(u8* dataBuffer, T*& fileSpacePointer)
		{
			fileSpacePointer = reinterpret_cast<T*>(&dataBuffer[reinterpret_cast<uintptr_t>(fileSpacePointer)]);
		}

		template <typename T>
		void RemapFileSpacePointerIfNotNull(u8* dataBuffer, T*& fileSpacePointer)
		{
			if (fileSpacePointer != nullptr)
				RemapFileSpacePointer(dataBuffer, fileSpacePointer);
		}

		void LinkDirectoryEntry(ComfyDirectory* directory, u8* dataBuffer)
		{
			RemapFileSpacePointerIfNotNull(dataBuffer, directory->Name);
			RemapFileSpacePointerIfNotNull(dataBuffer, directory->Entries);
			RemapFileSpacePointerIfNotNull(dataBuffer, directory->SubDirectories);

			for (size_t i = 0; i < directory->EntryCount; i++)
				RemapFileSpacePointerIfNotNull(dataBuffer, directory->Entries[i].Name);

			for (size_t i = 0; i < directory->SubDirectoryCount; i++)
				LinkDirectoryEntry(&directory->SubDirectories[i], dataBuffer);
		}
	}

	void ComfyArchive::LinkRemapPointers()
	{
		u8* offsetDataBuffer = dataBuffer.get() - header.DataOffset;

		rootDirectory = reinterpret_cast<ComfyDirectory*>(dataBuffer.get());
		LinkDirectoryEntry(rootDirectory, offsetDataBuffer);
	}

	void ComfyArchive::DecryptStrings()
	{
		// TODO:
	}
}
