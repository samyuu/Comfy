#include "FileReader.h"
#include "Core/Win32/ComfyWindows.h"

namespace FileSystem
{
	void* FileReader::CreateFileHandle(const std::string& filePath, bool read)
	{
		return ::CreateFileA(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, read ? FILE_SHARE_READ : NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	void* FileReader::CreateFileHandle(const std::wstring& filePath, bool read)
	{
		return ::CreateFileW(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, read ? FILE_SHARE_READ : NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	void FileReader::CloseFileHandle(void* fileHandle)
	{
		bool success = ::CloseHandle(fileHandle);
	}

	size_t FileReader::GetFileSize(void* fileHandle)
	{
		LARGE_INTEGER fileSizeLarge = {};
		bool success = ::GetFileSizeEx(fileHandle, &fileSizeLarge);

		return static_cast<size_t>(fileSizeLarge.QuadPart);
	}

	size_t FileReader::ReadFile(void* fileHandle, void* outputData, size_t dataSize)
	{
		DWORD bytesRead;
		bool success = ::ReadFile(fileHandle, outputData, static_cast<DWORD>(dataSize), &bytesRead, nullptr);

		return static_cast<size_t>(bytesRead);
	}
}