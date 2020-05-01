#include "File.h"
#include "Misc/UTF8.h"
#include "Core/Win32/ComfyWindows.h"

namespace Comfy::IO
{
	namespace File
	{
		bool Exists(std::string_view directoryPath)
		{
			const auto attributes = ::GetFileAttributesW(UTF8::WideArg(directoryPath).c_str());
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

		std::pair<UniquePtr<u8[]>, size_t> ReadAllBytes(std::string_view filePath)
		{
			std::pair<UniquePtr<u8[]>, size_t> result = {};
			auto&[fileContent, fileSize] = result;

			auto fileStream = OpenRead(filePath);
			if (!fileStream.IsOpen() || !fileStream.CanRead())
				return std::make_pair(nullptr, 0);

			fileSize = static_cast<size_t>(fileStream.GetLength());
			fileContent = MakeUnique<u8[]>(fileSize);

			if (fileContent == nullptr)
				return std::make_pair(nullptr, 0);

			fileStream.ReadBuffer(fileContent.get(), fileSize);
			return result;
		}

		bool ReadAllBytes(std::string_view filePath, std::vector<u8>& outFileContent)
		{
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
}
