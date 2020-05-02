#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include "Stream/FileStream.h"
#include "Stream/MemoryStream.h"
#include "Stream/Manipulator/StreamReader.h"
#include "Stream/Manipulator/StreamWriter.h"
#include "Stream/FileInterfaces.h"

namespace Comfy::IO
{
	namespace File
	{
		COMFY_NODISCARD bool Exists(std::string_view filePath);

		// NOTE: Use for mostly temporary variables, hence return by value
		COMFY_NODISCARD FileStream OpenRead(std::string_view filePath);
		COMFY_NODISCARD MemoryStream OpenReadMemory(std::string_view filePath);
		COMFY_NODISCARD FileStream CreateWrite(std::string_view filePath);

		COMFY_NODISCARD std::pair<UniquePtr<u8[]>, size_t> ReadAllBytes(std::string_view filePath);
		bool ReadAllBytes(std::string_view filePath, std::vector<u8>& outFileContent);

		COMFY_NODISCARD std::string ReadAllText(std::string_view filePath);

		bool WriteAllBytes(std::string_view filePath, const void* dataToWrite, size_t dataSize);
		bool WriteAllText(std::string_view filePath, std::string_view text);

		template <typename Readable>
		COMFY_NODISCARD UniquePtr<Readable> Load(std::string_view filePath)
		{
			static_assert(std::is_base_of_v<IStreamReadable, Readable>);

			auto stream = OpenReadMemory(filePath);
			if (!stream.IsOpen() || !stream.CanRead())
				return nullptr;

			auto result = MakeUnique<Readable>();
			if (result == nullptr)
				return nullptr;

			auto reader = StreamReader(stream);
			result->Read(reader);
			return result;
		}

		template <typename Writable>
		bool Save(std::string_view filePath, Writable& writable)
		{
			static_assert(std::is_base_of_v<IStreamWritable, Writable>);

			auto stream = CreateWrite(filePath);
			if (!stream.IsOpen() || !stream.CanWrite())
				return false;

			auto writer = StreamWriter(stream);
			writable.Write(writer);
			return true;
		}
	}

	// NOTE: Named 'Folder(-File)' to distinctly separate it from the 'Archive-' classes and directory.
	//		 The purpose of this code is to allow for seamless integration for files contain within other file archive / folder formats.
	//		 If the archive format is known in advance or more than one files are to be extracted at once, the corresponding archive class should be used directly instead.
	//		 For now only FArc files will be supported but future expansion should be possible without altering the interface.
	//
	// NOTE: The path syntax used is: {FolderFilePath}<{InternalFileName}>	- Example: "C:/MyDir/MyArc.farc<MyFile.ext>"
	//		 Nested archive paths will *not* be supported for now			- Example: "C:/MyDir/MyArc.farc<NestedArc.farc<NestedFile.ext>>" would be ill formatted
	//
	namespace FolderFile
	{
		constexpr char InternalFileStartMarker = '<', InternalFileEndMarker = '>';
		constexpr const char* InternalFileMarkers = "<>";

		struct FileEntry
		{
			std::string FullPath;
			size_t FileSize;
		};

		COMFY_NODISCARD constexpr std::pair<std::string_view, std::string_view> ParsePath(std::string_view folderFilePath)
		{
			const auto fileStartIndex = folderFilePath.find_first_of(InternalFileStartMarker);
			if (fileStartIndex == std::string_view::npos)
				return std::make_pair(folderFilePath, "");

			const auto fileEndIndex = folderFilePath.find_last_of(InternalFileEndMarker);
			if (fileEndIndex == std::string_view::npos || fileEndIndex < fileStartIndex)
				return std::make_pair(folderFilePath, "");

			const auto basePath = folderFilePath.substr(0, fileStartIndex);
			const auto internalFile = folderFilePath.substr(fileStartIndex + 1, fileEndIndex - fileStartIndex - 1);
			return std::make_pair(basePath, internalFile);
		}

		COMFY_NODISCARD std::string CombinePath(std::string_view basePath, std::string_view internalFile);

		COMFY_NODISCARD bool IsValidFolderFile(std::string_view basePath);

		COMFY_NODISCARD MemoryStream OpenReadInternalFileMemory(std::string_view basePath, std::string_view internalFile);
		
		COMFY_NODISCARD bool ReadAllBytesInternalFileMemory(std::string_view basePath, std::string_view internalFile, std::vector<u8>& outFileContent);

		COMFY_NODISCARD std::vector<FileEntry> GetFileEntries(std::string_view basePath);
	}
}
