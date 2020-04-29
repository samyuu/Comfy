#pragma once
#include "Misc/UTF8.h"
#include "Core/Win32/ComfyWindows.h"

namespace Comfy::IO
{
	inline void* CreateFileHandleInternal(std::string_view path, bool read)
	{
		return ::CreateFileW(UTF8::WideArg(path).c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	inline void CloseFileHandleInternal(HANDLE fileHandle)
	{
		::CloseHandle(fileHandle);
	}

	inline bool WriteAllBytesInternal(HANDLE fileHandle, const void* buffer, size_t bufferSize)
	{
		DWORD bytesToWrite = static_cast<DWORD>(bufferSize);
		DWORD bytesWritten = {};

		::WriteFile(fileHandle, buffer, bytesToWrite, &bytesWritten, nullptr);
		int error = ::GetLastError();

		return !FAILED(error) && (bytesWritten == bytesToWrite);
	}
}
