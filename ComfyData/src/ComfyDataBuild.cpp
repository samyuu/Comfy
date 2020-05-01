#include "IO/File.h"
#include "IO/Archive/ComfyArchive.h"
#include "IO/Stream/FileStream.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include "Core/Logger.h"
#include "Misc/StringHelper.h"
#include <filesystem>
#include <random>
#include <time.h>

using namespace Comfy;
using namespace Comfy::IO;

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

		std::vector<u8> FileContent;
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

		for (const auto& file : std::filesystem::directory_iterator(rootPath))
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
		FileAddr ReturnAddress;
		FileAddr DataAddress;
	};

	std::vector<TempFilePtrData> FileDataToWrite;

	void AppendWriteFileDataSizeAndPointer(StreamWriter& writer, Build_ComfyEntry& file)
	{
		FileDataToWrite.push_back({ &file, writer.GetPosition(), FileAddr::NullPtr });

		writer.WriteU32('ezis');
		writer.WriteU32(0);
		writer.WriteU32('trp');
		writer.WriteU32(0);
	}

	void WriteFileData(StreamWriter& writer)
	{
		for (auto& data : FileDataToWrite)
		{
			auto stream = IO::File::OpenRead(data.File->Build_OriginalPath.u8string());
			if (!stream.IsOpen() || !stream.CanRead())
				continue;

			data.File->FileContent.resize(static_cast<size_t>(stream.GetLength()));
			stream.ReadBuffer(data.File->FileContent.data(), data.File->FileContent.size());

			data.DataAddress = writer.GetPosition();
			writer.WriteBuffer(data.File->FileContent.data(), data.File->FileContent.size());
			writer.WritePadding(16);
		}

		for (auto& data : FileDataToWrite)
		{
			writer.SetPosition(data.ReturnAddress);
			writer.WriteU64(data.File->FileContent.size());
			writer.WritePtr(data.DataAddress);
		}
	}
}

namespace
{
	std::array<u8, 16> GetIV()
	{
		std::random_device randomDevice;
		std::mt19937 numberGenerator(randomDevice());

		constexpr u8 min = std::numeric_limits<u8>::min();
		constexpr u8 max = std::numeric_limits<u8>::max();

		std::uniform_int_distribution<std::mt19937::result_type> distribution(min, max);

		std::array<u8, 16> iv;
		for (auto& value : iv)
			value = distribution(numberGenerator);
		return iv;
	}

	void WriteHeaderBase(StreamWriter& writer)
	{
		// NOTE: Magic
		for (const auto value : ComfyArchive::Magic)
			writer.WriteU8(value);

		// NOTE: Version
		writer.WriteU8(ComfyArchive::Version.Major);
		writer.WriteU8(ComfyArchive::Version.Minor);

		// NOTE: ReservedVersion
		writer.WriteU16(0xCCCC);

		// NOTE: CreatorID
		std::array<u8, 4> creatorID = { 'c', 'm', 'f', 'y' };
		writer.WriteType_Native(creatorID);

		// NOTE: ReservedID
		std::array<u8, 4> reservedID = { 0x90, 0x90, 0x90, 0x90 };
		writer.WriteType_Native(reservedID);

		// NOTE: CreationDate
		__time64_t creationDate = time(0);
		writer.WriteU64(creationDate);

		// NOTE: Flags
		writer.WriteType_Native(ArchiveFlags);

		// NOTE: IV
		std::array<u8, 16> iv = GetIV();
		writer.WriteType_Native(iv);
	}

	void WriteFileEntries(StreamWriter& writer, Build_ComfyEntry& file)
	{
		// NOTE: Type
		writer.WriteType_Native(file.Type);
		// NOTE: Flags
		writer.WriteType_Native(file.Flags);

		// NOTE: Name
		writer.WriteStrPtr(file.Build_FileName);

		// NOTE: Size / Offset
		AppendWriteFileDataSizeAndPointer(writer, file);
	}

	void WriteDirectoryEntry(StreamWriter& writer, Build_ComfyDirectory& directory)
	{
		// NOTE: Type
		writer.WriteType_Native(directory.Type);
		// NOTE: Flags
		writer.WriteType_Native(directory.Flags);

		// NOTE: Name
		writer.WriteStrPtr(directory.Build_FileName);

		// NOTE: EntryCount
		writer.WriteU64(directory.Build_Entries.size());
		// NOTE: Entries
		if (directory.Build_Entries.empty())
		{
			writer.WritePtr(FileAddr::NullPtr);
		}
		else
		{
			writer.WriteFuncPtr([&](StreamWriter& writer)
			{
				for (auto& entry : directory.Build_Entries)
					WriteFileEntries(writer, entry);
			});
		}

		// NOTE: SubDirectoryCount
		writer.WriteU64(directory.Build_Directories.size());
		// NOTE: SubDirectories
		if (directory.Build_Directories.empty())
		{
			writer.WritePtr(FileAddr::NullPtr);
		}
		else
		{
			writer.WriteFuncPtr([&](StreamWriter& writer)
			{
				for (auto& entry : directory.Build_Directories)
					WriteDirectoryEntry(writer, entry);
			});
		}
	}

	void WriteFileTreeRoot(StreamWriter& writer)
	{
		// NOTE: Type
		writer.WriteType_Native(RootDirectory.Type);
		// NOTE: Flags
		writer.WriteType_Native(RootDirectory.Flags);

		// NOTE: Name
		writer.WriteStrPtr(RootDirectory.Build_FileName);

		// NOTE: EntryCount
		writer.WriteU64(RootDirectory.Build_Entries.size());
		// NOTE: Entries
		if (RootDirectory.Build_Entries.empty())
		{
			writer.WritePtr(FileAddr::NullPtr);
		}
		else
		{
			writer.WriteFuncPtr([&](StreamWriter& writer)
			{
				for (auto& entry : RootDirectory.Build_Entries)
					WriteFileEntries(writer, entry);
			});
		}

		// NOTE: SubDirectoryCount
		writer.WriteU64(RootDirectory.Build_Directories.size());
		// NOTE: SubDirectories
		if (RootDirectory.Build_Directories.empty())
		{
			writer.WritePtr(FileAddr::NullPtr);
		}
		else
		{
			writer.WriteFuncPtr([&](StreamWriter& writer)
			{
				for (auto& entry : RootDirectory.Build_Directories)
					WriteDirectoryEntry(writer, entry);
			});
		}
	}

	void WriteFileTree(StreamWriter& writer)
	{
		writer.SetPointerMode(PtrMode::Mode64Bit);

		writer.WriteU64(/*DataSize*/ 0x00);
		const FileAddr dataOffset = writer.GetPosition() + static_cast<FileAddr>(sizeof(dataOffset));
		writer.WritePtr(/*DataOffset*/ dataOffset);

		WriteFileTreeRoot(writer);
		writer.WritePtr(FileAddr::NullPtr);
		writer.WriteAlignmentPadding(16);

		writer.FlushPointerPool();
		writer.WriteAlignmentPadding(16);

		writer.WriteAlignmentPadding(16);

		writer.FlushDelayedWritePool();
		writer.FlushPointerPool();

		writer.FlushStringPointerPool();

		writer.SetPosition(static_cast<FileAddr>(offsetof(ComfyArchiveHeader, DataSize)));
		writer.WritePtr(writer.GetLength() - dataOffset);

		writer.SetPosition(writer.GetLength());
		writer.WriteAlignmentPadding(16);

		WriteFileData(writer);

		writer.SetPosition(writer.GetLength());
		writer.WriteAlignmentPadding(16);
	}

	int BuildArchive(std::string_view inputDirectoryPath, std::string_view outputArchivePath)
	{
		FileStream stream;
		stream.CreateWrite(outputArchivePath);
		StreamWriter writer(stream);

		ArchiveFlags.WideAddresses = true;
		ArchiveFlags.EncryptedStrings = false;
		ArchiveFlags.Verified = true;

		WriteHeaderBase(writer);
		BuildUpDirectoryTree(UTF8::Widen(inputDirectoryPath));

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

	const auto inputDirectoryPath = UTF8::Narrow(argv[1]);
	const auto outputArchivePath = UTF8::Narrow(argv[2]);

	if (!Directory::Exists(inputDirectoryPath))
	{
		Logger::LogErrorLine("Invalid directory input path");
		return EXIT_FAILURE;
	}

	return BuildArchive(inputDirectoryPath, outputArchivePath);
}
