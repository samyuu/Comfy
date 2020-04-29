#pragma once
#include "CoreTypes.h"

namespace Comfy::IO
{
	class FileReader
	{
	private:
		FileReader() = delete;

	public:
		template<typename T>
		static inline bool ReadEntireFile(std::string_view filePath, std::vector<T>* buffer)
		{
			auto fileHandle = CreateFileHandle(filePath, true);
			bool isValidHandle = reinterpret_cast<int64_t>(fileHandle) > 0;

			if (isValidHandle)
			{
				size_t fileSize = GetFileSize(fileHandle);
				buffer->resize((fileSize + sizeof(T) - 1) / sizeof(T));
				ReadFile(fileHandle, buffer->data(), fileSize);
				CloseFileHandle(fileHandle);
			}

			return isValidHandle;
		}

	private:
		static void* CreateFileHandle(std::string_view filePath, bool read);
		static void CloseFileHandle(void* fileHandle);

		static size_t GetFileSize(void* fileHandle);
		static size_t ReadFile(void* fileHandle, void* outputData, size_t dataSize);
	};
}
