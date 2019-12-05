#pragma once
#include "Core/Win32/ComfyWindows.h"

namespace FileSystem
{
	template <typename T>
	inline auto NullTerminatedPathBufferInternal(std::basic_string_view<T> path)
	{
		const size_t pathCharacterCount = std::clamp(path.size(), static_cast<size_t>(1), MAX_PATH * sizeof(T));

		std::array<T, MAX_PATH> pathBuffer;
		std::memcpy(pathBuffer.data(), path.data(), (pathCharacterCount) * sizeof(T));

		pathBuffer[pathCharacterCount] = 0;

		return pathBuffer;
	}

	template <typename T>
	inline auto CreateFileHandleInternal(std::basic_string_view<T> path, bool read)
	{
		auto pathBuffer = NullTerminatedPathBufferInternal(path);

		if constexpr (std::is_same<T, char>::value)
			return ::CreateFileA(pathBuffer.data(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		else
			return ::CreateFileW(pathBuffer.data(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	inline void CloseFileHandleInternal(HANDLE fileHandle)
	{
		::CloseHandle(fileHandle);
	}

	inline bool WriteAllBytesInternal(HANDLE fileHandle, const std::vector<uint8_t>& buffer)
	{
		DWORD bytesToWrite = static_cast<DWORD>(buffer.size());
		DWORD bytesWritten = {};

		::WriteFile(fileHandle, buffer.data(), bytesToWrite, &bytesWritten, nullptr);
		int error = ::GetLastError();

		return !FAILED(error) && (bytesWritten == bytesToWrite);
	}
}
