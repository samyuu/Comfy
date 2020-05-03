#include "File.h"
#include "Path.h"
#include "Archive/FArc.h"
#include "Misc/UTF8.h"
#include "Core/Win32/ComfyWindows.h"

namespace Comfy::IO
{
	namespace File
	{
		bool Exists(std::string_view filePath)
		{
			const auto[basePath, internalFile] = FolderFile::ParsePath(filePath);

			const auto attributes = ::GetFileAttributesW(UTF8::WideArg(basePath).c_str());
			return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
		}

		FileStream OpenRead(std::string_view filePath)
		{
			FileStream result;
			result.OpenRead(filePath);
			return result;
		}

		MemoryStream OpenReadMemory(std::string_view filePath)
		{
			if (const auto[basePath, internalFile] = FolderFile::ParsePath(filePath); !internalFile.empty())
				return FolderFile::OpenReadInternalFileMemory(basePath, internalFile);

			FileStream fileStream = OpenRead(filePath);
			if (!fileStream.IsOpen() || !fileStream.CanRead())
				return MemoryStream();

			MemoryStream result;
			result.FromStream(fileStream);
			return result;
		}

		FileStream CreateWrite(std::string_view filePath)
		{
			FileStream result;
			result.OpenWrite(filePath);
			return result;
		}

		std::pair<std::unique_ptr<u8[]>, size_t> ReadAllBytes(std::string_view filePath)
		{
			if (const auto[basePath, internalFile] = FolderFile::ParsePath(filePath); !internalFile.empty())
				return FolderFile::ReadAllBytesInternalFileMemory(basePath, internalFile);

			std::pair<std::unique_ptr<u8[]>, size_t> result = {};
			auto&[fileContent, fileSize] = result;

			auto fileStream = OpenRead(filePath);
			if (!fileStream.IsOpen() || !fileStream.CanRead())
				return std::make_pair(nullptr, 0);

			fileSize = static_cast<size_t>(fileStream.GetLength());
			fileContent = std::make_unique<u8[]>(fileSize);

			if (fileContent == nullptr)
				return std::make_pair(nullptr, 0);

			fileStream.ReadBuffer(fileContent.get(), fileSize);
			return result;
		}

		bool ReadAllBytes(std::string_view filePath, std::vector<u8>& outFileContent)
		{
			if (const auto[basePath, internalFile] = FolderFile::ParsePath(filePath); !internalFile.empty())
				return FolderFile::ReadAllBytesInternalFileMemory(basePath, internalFile, outFileContent);

			auto fileStream = OpenRead(filePath);
			if (!fileStream.IsOpen() || !fileStream.CanRead())
				return false;

			outFileContent.resize(static_cast<size_t>(fileStream.GetLength()));
			fileStream.ReadBuffer(outFileContent.data(), outFileContent.size());
			return true;
		}

		std::string ReadAllText(std::string_view filePath)
		{
			std::string result;
			auto fileStream = OpenRead(filePath);
			if (!fileStream.IsOpen() || !fileStream.CanRead())
				return result;

			result.resize(static_cast<size_t>(fileStream.GetLength()));
			fileStream.ReadBuffer(result.data(), result.size());
			return result;
		}

		bool WriteAllBytes(std::string_view filePath, const void* dataToWrite, size_t dataSize)
		{
			auto fileStream = CreateWrite(filePath);
			if (!fileStream.IsOpen() || !fileStream.CanWrite())
				return false;

			fileStream.WriteBuffer(dataToWrite, dataSize);
			return true;
		}

		bool WriteAllText(std::string_view filePath, std::string_view text)
		{
			return WriteAllBytes(filePath, text.data(), text.size());
		}
	}

	namespace FolderFile
	{
		std::string CombinePath(std::string_view basePath, std::string_view internalFile)
		{
			auto result = std::string(basePath);
			if (internalFile.empty())
				return result;

			result.reserve(basePath.size() + internalFile.size() + 2);
			result += InternalFileStartMarker;
			result += internalFile;
			result += InternalFileEndMarker;
			return result;
		}

		bool IsValidFolderFile(std::string_view basePath)
		{
			return Path::GetExtension(basePath) == ".farc";
		}

		MemoryStream OpenReadInternalFileMemory(std::string_view basePath, std::string_view internalFile)
		{
			MemoryStream result;
			if (Path::GetExtension(basePath) == ".farc")
			{
				if (auto farc = FArc::Open(basePath); farc)
				{
					if (const auto file = farc->FindFile(internalFile); file != nullptr)
						result.FromBuffer(file->OriginalSize, [&](void* outContent, size_t size) { file->ReadIntoBuffer(outContent); });
				}
			}
			return result;
		}

		std::pair<std::unique_ptr<u8[]>, size_t> ReadAllBytesInternalFileMemory(std::string_view basePath, std::string_view internalFile)
		{
			std::pair<std::unique_ptr<u8[]>, size_t> result = {};
			auto&[fileContent, fileSize] = result;

			if (Path::GetExtension(basePath) == ".farc")
			{
				if (auto farc = FArc::Open(basePath); farc)
				{
					if (const auto file = farc->FindFile(internalFile); file != nullptr)
					{
						fileContent = std::make_unique<u8[]>(file->OriginalSize);
						fileSize = file->OriginalSize;
						
						file->ReadIntoBuffer(fileContent.get());
						return result;
					}
				}
			}
			return result;
		}

		bool ReadAllBytesInternalFileMemory(std::string_view basePath, std::string_view internalFile, std::vector<u8>& outFileContent)
		{
			if (Path::GetExtension(basePath) == ".farc")
			{
				if (auto farc = FArc::Open(basePath); farc)
				{
					if (const auto file = farc->FindFile(internalFile); file != nullptr)
					{
						outFileContent.resize(file->OriginalSize);
						file->ReadIntoBuffer(outFileContent.data());
						return true;
					}
				}
			}
			return false;
		}

		std::vector<FileEntry> GetFileEntries(std::string_view basePath)
		{
			auto result = std::vector<FileEntry>();
			if (Path::GetExtension(basePath) == ".farc")
			{
				if (auto farc = FArc::Open(basePath); farc)
				{
					result.reserve(farc->GetEntries().size());
					for (auto file : farc->GetEntries())
					{
						auto& newEntry = result.emplace_back();
						newEntry.FullPath = std::move(CombinePath(basePath, file.Name));
						newEntry.FileSize = file.OriginalSize;
					}
				}
			}
			return result;
		}
	}
}
