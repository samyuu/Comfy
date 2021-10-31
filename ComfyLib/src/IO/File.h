#pragma once
#include "Types.h"
#include "Stream/FileStream.h"
#include "Stream/MemoryStream.h"
#include "Stream/Manipulator/StreamReader.h"
#include "Stream/Manipulator/StreamWriter.h"
#include "Stream/FileInterfaces.h"
#include <future>

namespace Comfy::IO
{
	namespace File
	{
		COMFY_NODISCARD bool Exists(std::string_view filePath);
		bool Copy(std::string_view source, std::string_view destination, bool overwriteExisting = false);

		// NOTE: Use for mostly temporary variables, hence return by value
		COMFY_NODISCARD FileStream OpenRead(std::string_view filePath);
		COMFY_NODISCARD MemoryStream OpenReadMemory(std::string_view filePath);
		COMFY_NODISCARD FileStream CreateWrite(std::string_view filePath);

		// NOTE: Prefer unique_ptr overload to avoid having to zero clear the vector when resizing
		COMFY_NODISCARD std::pair<std::unique_ptr<u8[]>, size_t> ReadAllBytes(std::string_view filePath);
		bool ReadAllBytes(std::string_view filePath, std::vector<u8>& outFileContent);

		COMFY_NODISCARD std::string ReadAllText(std::string_view filePath);

		bool WriteAllBytes(std::string_view filePath, const void* dataToWrite, size_t dataSize);
		bool WriteAllText(std::string_view filePath, std::string_view text);

		template <typename Readable>
		COMFY_NODISCARD std::unique_ptr<Readable> LoadStreamReadable(std::string_view filePath)
		{
			static_assert(std::is_base_of_v<IStreamReadable, Readable>);

			auto stream = OpenReadMemory(filePath);
			if (!stream.IsOpen() || !stream.CanRead())
				return nullptr;

			auto result = std::make_unique<Readable>();
			if (result == nullptr)
				return nullptr;

			auto reader = StreamReader(stream);

			if (const auto streamResult = result->Read(reader); streamResult != StreamResult::Success)
				return nullptr;

			return result;
		}

		template <typename Parsable>
		COMFY_NODISCARD std::unique_ptr<Parsable> LoadBufferParsable(std::string_view filePath)
		{
			static_assert(std::is_base_of_v<IBufferParsable, Parsable>);

			const auto[fileContent, fileSize] = ReadAllBytes(filePath);
			if (fileContent == nullptr)
				return nullptr;

			auto result = std::make_unique<Parsable>();
			if (result == nullptr)
				return nullptr;

			result->Parse(fileContent.get(), fileSize);
			return result;
		}

		template <typename Loadable>
		COMFY_NODISCARD std::unique_ptr<Loadable> Load(std::string_view filePath)
		{
			if constexpr (std::is_base_of_v<IStreamReadable, Loadable>)
				return LoadStreamReadable<Loadable>(filePath);
			else if constexpr (std::is_base_of_v<IBufferParsable, Loadable>)
				return LoadBufferParsable<Loadable>(filePath);
			else
				static_assert(false, "Unable to load class type");
		}

		template <typename Writable>
		bool Save(std::string_view filePath, Writable& writable)
		{
			static_assert(std::is_base_of_v<IStreamWritable, Writable>);

			auto stream = CreateWrite(filePath);
			if (!stream.IsOpen() || !stream.CanWrite())
				return false;

			auto writer = StreamWriter(stream);
			if (const auto streamResult = writable.Write(writer); streamResult != StreamResult::Success)
				return false;

			return true;
		}

		template <typename Loadable>
		COMFY_NODISCARD std::future<std::unique_ptr<Loadable>> LoadAsync(std::string_view filePath)
		{
			return std::async(std::launch::async, [path = std::string(filePath)] { return Load<Loadable>(path); });
		}

		// NOTE: The writable input parameter must outlive the duration of the returned future!
		template <typename Writable>
		COMFY_NODISCARD std::future<bool> SaveAsync(std::string_view filePath, Writable* writable)
		{
			return std::async(std::launch::async, [path = std::string(filePath), writable] { return (writable != nullptr) ? Save<Writable>(path, *writable) : false; });
		}
	}

	// NOTE: The purpose of this code is to allow for seamless integration for files contain within other file archive formats.
	//		 If the archive format is known in advance or more than one files are to be extracted at once, the corresponding archive class should be used directly instead.
	//		 For now only FArc files will be supported but future expansion should be possible without altering the interface.
	//
	// NOTE: The path syntax used is: {ArchiveFilePath}<{FileName}>			- Example: "C:/MyDir/MyArc.farc<MyFile.ext>"
	//		 Nested archive paths will *not* be supported for now			- Example: "C:/MyDir/MyArc.farc<NestedArc.farc<NestedFile.ext>>" would be ill formatted
	//
	namespace Archive
	{
		namespace Detail
		{
			struct FileEntry
			{
				std::string FullPath;
				size_t FileSize;
			};

			COMFY_NODISCARD MemoryStream ToMemoryStream(std::string_view basePath, std::string_view fileName);

			COMFY_NODISCARD std::pair<std::unique_ptr<u8[]>, size_t> ReadAllBytes(std::string_view basePath, std::string_view fileName);

			COMFY_NODISCARD bool ReadAllBytes(std::string_view basePath, std::string_view fileName, std::vector<u8>& outFileContent);

			COMFY_NODISCARD std::vector<FileEntry> GetFileEntries(std::string_view basePath);
		}

		constexpr char FileStartMarker = '<', FileEndMarker = '>';
		constexpr const char* FileMarkers = "<>";

		struct ArchivePath
		{
			std::string_view BasePath;
			std::string_view FileName;
		};

		COMFY_NODISCARD constexpr ArchivePath ParsePath(std::string_view archiveFilePath)
		{
			const auto fileStartIndex = archiveFilePath.find_first_of(FileStartMarker);
			if (fileStartIndex == std::string_view::npos)
				return { archiveFilePath, "" };

			const auto fileEndIndex = archiveFilePath.find_last_of(FileEndMarker);
			if (fileEndIndex == std::string_view::npos || fileEndIndex < fileStartIndex)
				return { archiveFilePath, "" };

			const auto basePath = archiveFilePath.substr(0, fileStartIndex);
			const auto fileName = archiveFilePath.substr(fileStartIndex + 1, fileEndIndex - fileStartIndex - 1);
			return { basePath, fileName };
		}

		COMFY_NODISCARD std::string CombinePath(std::string_view basePath, std::string_view fileName);

		COMFY_NODISCARD bool IsValidPath(std::string_view basePath);
	}
}
