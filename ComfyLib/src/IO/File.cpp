#include "File.h"
#include "Path.h"
#include "Archive/FArc.h"
#include "Misc/UTF8.h"
#include "Core/Win32LeanWindowsHeader.h"

namespace Comfy::IO
{
	namespace File
	{
		bool Exists(std::string_view filePath)
		{
			const auto archivePath = Archive::ParsePath(filePath);

			const auto attributes = ::GetFileAttributesW(UTF8::WideArg(archivePath.BasePath).c_str());
			return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
		}

		bool Copy(std::string_view source, std::string_view destination, bool overwriteExisting)
		{
			const bool failIfExists = !overwriteExisting;
			const auto success = ::CopyFileW(UTF8::WideArg(source).c_str(), UTF8::WideArg(destination).c_str(), failIfExists);

			return (success != 0);
		}

		FileStream OpenRead(std::string_view filePath)
		{
			FileStream result;
			result.OpenRead(filePath);
			return result;
		}

		MemoryStream OpenReadMemory(std::string_view filePath)
		{
			if (const auto archivePath = Archive::ParsePath(filePath); !archivePath.FileName.empty())
				return Archive::Detail::ToMemoryStream(archivePath.BasePath, archivePath.FileName);

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
			result.CreateWrite(filePath);
			return result;
		}

		std::pair<std::unique_ptr<u8[]>, size_t> ReadAllBytes(std::string_view filePath)
		{
			if (const auto archivePath = Archive::ParsePath(filePath); !archivePath.FileName.empty())
				return Archive::Detail::ReadAllBytes(archivePath.BasePath, archivePath.FileName);

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
			if (const auto archivePath = Archive::ParsePath(filePath); !archivePath.FileName.empty())
				return Archive::Detail::ReadAllBytes(archivePath.BasePath, archivePath.FileName, outFileContent);

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

	namespace Archive
	{
		namespace Detail
		{
			namespace FArcImpl
			{
				bool IsValidPath(std::string_view basePath)
				{
					return Path::GetExtension(basePath) == ".farc";
				}

				bool ToMemoryStream(std::string_view basePath, std::string_view fileName, MemoryStream& outStream)
				{
					auto farc = FArc::Open(basePath);
					if (farc == nullptr)
						return false;

					const auto file = farc->FindFile(fileName);
					if (file == nullptr)
						return false;

					outStream.FromBuffer(file->OriginalSize, [&](void* outContent, size_t size) { file->ReadIntoBuffer(outContent); });
					return true;
				}

				template <typename SizeCallback, typename BufferGetter>
				bool ReadFile(std::string_view basePath, std::string_view fileName, SizeCallback sizeCallback, BufferGetter bufferGetter)
				{
					auto farc = FArc::Open(basePath);
					if (farc == nullptr)
						return false;

					const auto file = farc->FindFile(fileName);
					if (file == nullptr)
						return false;

					sizeCallback(file->OriginalSize);
					void* outContent = bufferGetter();

					if (outContent == nullptr)
						return false;

					file->ReadIntoBuffer(outContent);
					return true;
				}

				bool GetFileEntries(std::string_view basePath, std::vector<Detail::FileEntry>& outEntries)
				{
					auto farc = FArc::Open(basePath);
					if (farc == nullptr)
						return false;

					outEntries.reserve(farc->GetEntries().size());
					for (auto file : farc->GetEntries())
					{
						auto& newEntry = outEntries.emplace_back();
						newEntry.FullPath = std::move(CombinePath(basePath, file.Name));
						newEntry.FileSize = file.OriginalSize;
					}

					return true;
				}
			}

			MemoryStream ToMemoryStream(std::string_view basePath, std::string_view fileName)
			{
				MemoryStream result;
				if (FArcImpl::IsValidPath(basePath))
					FArcImpl::ToMemoryStream(basePath, fileName, result);
				return result;
			}

			std::pair<std::unique_ptr<u8[]>, size_t> ReadAllBytes(std::string_view basePath, std::string_view fileName)
			{
				std::pair<std::unique_ptr<u8[]>, size_t> result = {};
				if (FArcImpl::IsValidPath(basePath))
				{
					FArcImpl::ReadFile(basePath, fileName,
						[&](size_t fileSize) { result.first = std::make_unique<u8[]>(result.second = fileSize); },
						[&]() { return result.first.get(); });
				}
				return result;
			}

			bool ReadAllBytes(std::string_view basePath, std::string_view fileName, std::vector<u8>& outFileContent)
			{
				if (FArcImpl::IsValidPath(basePath))
				{
					return FArcImpl::ReadFile(basePath, fileName,
						[&](size_t fileSize) { outFileContent.resize(fileSize); },
						[&]() { return outFileContent.data(); });
				}
				return false;
			}

			std::vector<FileEntry> GetFileEntries(std::string_view basePath)
			{
				auto result = std::vector<FileEntry>();
				if (FArcImpl::IsValidPath(basePath))
					FArcImpl::GetFileEntries(basePath, result);
				return result;
			}
		}

		std::string CombinePath(std::string_view basePath, std::string_view fileName)
		{
			auto result = std::string(basePath);
			if (fileName.empty())
				return result;

			result.reserve(basePath.size() + fileName.size() + 2);
			result += FileStartMarker;
			result += fileName;
			result += FileEndMarker;
			return result;
		}

		bool IsValidPath(std::string_view basePath)
		{
			return Detail::FArcImpl::IsValidPath(basePath);
		}
	}
}
