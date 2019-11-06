#include "FileSystem/FileHelper.h"
#include "FileSystem/Archive/ComfyArchive.h"
#include "FileSystem/Stream/FileStream.h"
#include "FileSystem/BinaryWriter.h"
#include "Core/Logger.h"
#include <filesystem>
#include <random>
#include <time.h>

using namespace FileSystem;

namespace
{
	ComfyArchiveFlags ArchiveFlags = {};
	
	void EncryptString(std::string& string)
	{
		if (!ArchiveFlags.EncryptedStrings)
			return;

		// TODO:
		for (auto& c : string)
			c ^= 0xCC;
	}
}

namespace
{
	struct Build_ComfyEntry : ComfyEntry
	{
		std::filesystem::path Build_OriginalPath;
		std::string Build_FileName;

		std::vector<uint8_t> FileContent;
	};

	struct Build_ComfyDirectory : ComfyDirectory
	{
		std::filesystem::path Build_OriginalPath;
		std::string Build_FileName;

		std::vector<Build_ComfyEntry> Build_Entries;
		std::vector<Build_ComfyDirectory> Build_Directories;
	};

	Build_ComfyDirectory RootDirectory;

	void RegisterFile(Build_ComfyDirectory& parent, const std::filesystem::path& path)
	{
		Build_ComfyEntry entry = {};
		{
			entry.Type = EntryType::File;
			entry.Build_OriginalPath = path;
			entry.Build_FileName = path.filename().u8string();
			entry.Flags.Verified = true;
			EncryptString(entry.Build_FileName);
		}
		parent.Build_Entries.push_back(entry);
	}

	void RegisterDirectory(Build_ComfyDirectory& parent, const std::filesystem::path& path)
	{
		Build_ComfyDirectory entry = {};
		entry.Type = EntryType::Directory;
		entry.Build_OriginalPath = path;
		entry.Build_FileName = path.filename().u8string();
		entry.Flags.Verified = true;
		EncryptString(entry.Build_FileName);

		for (auto& file : std::filesystem::directory_iterator(path))
		{
			if (file.is_directory())
				RegisterDirectory(entry, file);
			else if (file.is_regular_file())
				RegisterFile(entry, file);
		}

		parent.Build_Directories.push_back(entry);
	}

	void BuildUpDirectoryTree(const std::filesystem::path& rootPath)
	{
		RootDirectory.Type = EntryType::Root;
		RootDirectory.Build_OriginalPath = rootPath;
		RootDirectory.Build_FileName = rootPath.filename().u8string();
		RootDirectory.Flags.Verified = true;
		EncryptString(RootDirectory.Build_FileName);

		for (auto& file : std::filesystem::directory_iterator(rootPath))
		{
			if (file.is_directory())
				RegisterDirectory(RootDirectory, file);
			else if (file.is_regular_file())
				RegisterFile(RootDirectory, file);
		}
	}
}

namespace
{
	struct TempFilePtrData
	{
		Build_ComfyEntry* File;
		void* ReturnAddress;
		void* DataAddress;
	};

	std::vector<TempFilePtrData> FileDataToWrite;

	void AppendWriteFileDataSizeAndPointer(BinaryWriter& writer, Build_ComfyEntry& file)
	{
		FileDataToWrite.push_back({ &file, writer.GetPositionPtr(), nullptr });

		writer.WriteUInt32('ezis');
		writer.WriteUInt32(0);
		writer.WriteUInt32('trp');
		writer.WriteUInt32(0);
	}

	void WriteFileData(BinaryWriter& writer)
	{
		for (auto& data : FileDataToWrite)
		{
			FileReader::ReadEntireFile(data.File->Build_OriginalPath.wstring(), &data.File->FileContent);

			data.DataAddress = writer.GetPositionPtr();
			writer.Write(data.File->FileContent.data(), data.File->FileContent.size());
			writer.WritePadding(16);
		}

		for (auto& data : FileDataToWrite)
		{
			writer.SetPosition(data.ReturnAddress);
			writer.WriteUInt64(data.File->FileContent.size());
			writer.WritePtr(data.DataAddress);
		}
	}
}

namespace
{
	std::array<uint8_t, 16> GetIV()
	{
		std::random_device randomDevice;
		std::mt19937 numberGenerator(randomDevice());

		constexpr uint8_t min = std::numeric_limits<uint8_t>::min();
		constexpr uint8_t max = std::numeric_limits<uint8_t>::max();

		std::uniform_int_distribution<std::mt19937::result_type> distribution(min, max);

		std::array<uint8_t, 16> iv;
		for (auto& value : iv)
			value = distribution(numberGenerator);
		return iv;
	}

	void WriteHeaderBase(BinaryWriter& writer)
	{
		// NOTE: Magic
		for (auto value : ComfyArchive::Magic)
			writer.WriteUInt8(value);

		// NOTE: Version
		writer.WriteUInt8(ComfyArchive::Version.Major);
		writer.WriteUInt8(ComfyArchive::Version.Minor);

		// NOTE: ReservedVersion
		writer.WriteUInt16(0xCCCC);

		// NOTE: CreatorID
		std::array<uint8_t, 4> creatorID = { 'c', 'm', 'f', 'y' };
		writer.Write(creatorID);

		// NOTE: ReservedID
		std::array<uint8_t, 4> reservedID = { 0x90, 0x90, 0x90, 0x90 };
		writer.Write(reservedID);

		// NOTE: CreationDate
		__time64_t creationDate = time(0);
		writer.WriteUInt64(creationDate);

		// NOTE: Flags
		writer.Write(ArchiveFlags);

		// NOTE: IV
		std::array<uint8_t, 16> iv = GetIV();
		writer.Write(iv);
	}

	void WriteFileEntries(BinaryWriter& writer, Build_ComfyEntry& file)
	{
		// NOTE: Type
		writer.Write(file.Type);
		// NOTE: Flags
		writer.Write(file.Flags);

		// NOTE: Name
		writer.WriteStrPtr(&file.Build_FileName);

		// NOTE: Size / Offset
		AppendWriteFileDataSizeAndPointer(writer, file);
	}

	void WriteDirectoryEntry(BinaryWriter& writer, Build_ComfyDirectory& directory)
	{
		// NOTE: Type
		writer.Write(directory.Type);
		// NOTE: Flags
		writer.Write(directory.Flags);

		// NOTE: Name
		writer.WriteStrPtr(&directory.Build_FileName);

		// NOTE: EntryCount
		writer.WriteUInt64(directory.Build_Entries.size());
		// NOTE: Entries
		if (directory.Build_Entries.empty())
		{
			writer.WritePtr(nullptr);
		}
		else
		{
			writer.WritePtr([&](BinaryWriter& writer)
			{
				for (auto& entry : directory.Build_Entries)
					WriteFileEntries(writer, entry);
			});
		}

		// NOTE: SubDirectoryCount
		writer.WriteUInt64(directory.Build_Directories.size());
		// NOTE: SubDirectories
		if (directory.Build_Directories.empty())
		{
			writer.WritePtr(nullptr);
		}
		else
		{
			writer.WritePtr([&](BinaryWriter& writer)
			{
				for (auto& entry : directory.Build_Directories)
					WriteDirectoryEntry(writer, entry);
			});
		}
	}

	void WriteFileTreeRoot(BinaryWriter& writer)
	{
		// NOTE: Type
		writer.Write(RootDirectory.Type);
		// NOTE: Flags
		writer.Write(RootDirectory.Flags);

		// NOTE: Name
		writer.WriteStrPtr(&RootDirectory.Build_FileName);

		// NOTE: EntryCount
		writer.WriteUInt64(RootDirectory.Build_Entries.size());
		// NOTE: Entries
		if (RootDirectory.Build_Entries.empty())
		{
			writer.WritePtr(nullptr);
		}
		else
		{
			writer.WritePtr([&](BinaryWriter& writer)
			{
				for (auto& entry : RootDirectory.Build_Entries)
					WriteFileEntries(writer, entry);
			});
		}

		// NOTE: SubDirectoryCount
		writer.WriteUInt64(RootDirectory.Build_Directories.size());
		// NOTE: SubDirectories
		if (RootDirectory.Build_Directories.empty())
		{
			writer.WritePtr(nullptr);
		}
		else
		{
			writer.WritePtr([&](BinaryWriter& writer)
			{
				for (auto& entry : RootDirectory.Build_Directories)
					WriteDirectoryEntry(writer, entry);
			});
		}
	}

	void WriteFileTree(BinaryWriter& writer)
	{
		writer.SetPointerMode(PtrMode::Mode64Bit);

		writer.WriteUInt64(/*DataSize*/ 0x00);
		const uint64_t dataOffset = writer.GetPosition() + sizeof(dataOffset);
		writer.WriteUInt64(/*DataOffset*/ dataOffset);

		WriteFileTreeRoot(writer);
		writer.WritePtr(nullptr);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.WriteAlignmentPadding(16);

		writer.FlushDelayedWritePool();
		writer.FlushPointerPool();

		writer.FlushStringPointerPool();

		writer.SetPosition(offsetof(ComfyArchiveHeader, DataSize));
		writer.WriteUInt64(writer.GetLength() - dataOffset);

		writer.SetPosition(writer.GetLength());
		writer.WriteAlignmentPadding(16);

		WriteFileData(writer);

		writer.SetPosition(writer.GetLength());
		writer.WriteAlignmentPadding(16);
	}

	int BuildArchive(const std::wstring& inputDirectoryPath, const std::wstring& outputArchivePath)
	{
		FileStream stream;
		stream.CreateWrite(outputArchivePath);
		BinaryWriter writer(&stream);

		ArchiveFlags.WideAddresses = true;
		ArchiveFlags.EncryptedStrings = false;
		ArchiveFlags.Verified = true;

		WriteHeaderBase(writer);
		BuildUpDirectoryTree(inputDirectoryPath);

		WriteFileTree(writer);

		return EXIT_SUCCESS;
	}
}

int wmain(int argc, const wchar_t* argv[])
{
	if (argc < 3)
	{
		Logger::LogErrorLine("Insufficient number of arguments");
		return EXIT_FAILURE;
	}

	const std::wstring inputDirectoryPath = argv[1];
	const std::wstring outputArchivePath = argv[2];

	if (!DirectoryExists(inputDirectoryPath))
	{
		Logger::LogErrorLine("Invalid directory input path");
		return EXIT_FAILURE;
	}

	return BuildArchive(inputDirectoryPath, outputArchivePath);
}