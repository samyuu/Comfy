#include "FileStream.h"
#include "Core/Win32/ComfyWindows.h"
#include "Misc/StringHelper.h"

namespace Comfy::IO
{
	FileStream::FileStream(FileStream&& other) : FileStream()
	{
		canRead = other.canRead;
		canWrite = other.canWrite;
		position = other.position;
		fileSize = other.fileSize;
		fileHandle = other.fileHandle;

		other.canRead = false;
		other.canWrite = false;
		other.position = {};
		other.fileSize = {};
		other.fileHandle = nullptr;
	}

	FileStream::~FileStream()
	{
		Close();
	}

	void FileStream::Seek(FileAddr position)
	{
		::LARGE_INTEGER distanceToMove;
		distanceToMove.QuadPart = static_cast<LONGLONG>(position);
		::SetFilePointerEx(fileHandle, distanceToMove, NULL, FILE_BEGIN);

		this->position = position;
	}

	FileAddr FileStream::GetPosition() const
	{
		return position;
	}

	FileAddr FileStream::GetLength() const
	{
		return fileSize;
	}

	bool FileStream::IsOpen() const
	{
		return fileHandle != INVALID_HANDLE_VALUE;
	}

	bool FileStream::CanRead() const
	{
		return canRead;
	}

	bool FileStream::CanWrite() const
	{
		return canWrite;
	}

	size_t FileStream::ReadBuffer(void* buffer, size_t size)
	{
		assert(canRead);

		DWORD bytesRead = 0;
		::ReadFile(fileHandle, buffer, static_cast<DWORD>(size), &bytesRead, nullptr);

		position += static_cast<FileAddr>(bytesRead);
		return bytesRead;
	}

	size_t FileStream::WriteBuffer(const void* buffer, size_t size)
	{
		assert(canWrite);

		DWORD bytesWritten = 0;
		::WriteFile(fileHandle, buffer, static_cast<DWORD>(size), &bytesWritten, nullptr);

		if (position > (GetLength() - static_cast<FileAddr>(bytesWritten)))
			fileSize += static_cast<FileAddr>(bytesWritten);
		position += static_cast<FileAddr>(bytesWritten);

		return bytesWritten;
	}

	void FileStream::OpenRead(std::string_view filePath)
	{
		assert(fileHandle == nullptr || fileHandle == INVALID_HANDLE_VALUE);
		fileHandle = ::CreateFileW(UTF8::WideArg(filePath).c_str(), (GENERIC_READ), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle != INVALID_HANDLE_VALUE)
			canRead = true;
		UpdateFileSize();
	}

	void FileStream::OpenWrite(std::string_view filePath)
	{
		assert(fileHandle == nullptr || fileHandle == INVALID_HANDLE_VALUE);
		fileHandle = ::CreateFileW(UTF8::WideArg(filePath).c_str(), (GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle != INVALID_HANDLE_VALUE)
			canWrite = true;
		UpdateFileSize();
	}

	void FileStream::OpenReadWrite(std::string_view filePath)
	{
		assert(fileHandle == nullptr || fileHandle == INVALID_HANDLE_VALUE);
		fileHandle = ::CreateFileW(UTF8::WideArg(filePath).c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			canRead = true;
			canWrite = true;
		}
		UpdateFileSize();
	}

	void FileStream::CreateWrite(std::string_view filePath)
	{
		assert(fileHandle == nullptr || fileHandle == INVALID_HANDLE_VALUE);
		fileHandle = ::CreateFileW(UTF8::WideArg(filePath).c_str(), (GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle != INVALID_HANDLE_VALUE)
			canWrite = true;
		UpdateFileSize();
	}

	void FileStream::CreateReadWrite(std::string_view filePath)
	{
		assert(fileHandle == nullptr || fileHandle == INVALID_HANDLE_VALUE);
		fileHandle = ::CreateFileW(UTF8::WideArg(filePath).c_str(), (GENERIC_READ | GENERIC_WRITE), (FILE_SHARE_READ | FILE_SHARE_WRITE), NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			canRead = true;
			canWrite = true;
		}
		UpdateFileSize();
	}

	void FileStream::Close()
	{
		if (fileHandle != INVALID_HANDLE_VALUE)
			::CloseHandle(fileHandle);
		fileHandle = nullptr;
	}

	void FileStream::UpdateFileSize()
	{
		if (fileHandle != INVALID_HANDLE_VALUE)
		{
			::LARGE_INTEGER largeIntegerFileSize = {};
			::GetFileSizeEx(fileHandle, &largeIntegerFileSize);

			fileSize = static_cast<FileAddr>(largeIntegerFileSize.QuadPart);
		}
		else
		{
			fileSize = {};
			canRead = false;
			canWrite = false;
		}
	}
}
