#include "FileStream.h"
#include <Windows.h>
#include <assert.h>

namespace FileSystem
{
	FileStream::FileStream()
	{
	}

	FileStream::FileStream(const std::wstring& filePath)
	{
		OpenReadWrite(filePath);
	}

	FileStream::~FileStream()
	{
	}

	void FileStream::Seek(int64_t position)
	{
		LARGE_INTEGER distanceToMove;
		distanceToMove.QuadPart = 0;
		SetFilePointerEx(fileHandle, distanceToMove, NULL, FILE_BEGIN);

		fileSize = __min(position, GetLength());
	}

	int64_t FileStream::GetPosition() const
	{
		return position;
	}

	int64_t FileStream::GetLength() const
	{
		return fileSize;
	}

	bool FileStream::IsOpen() const
	{
		return fileHandle != nullptr;
	}

	bool FileStream::CanRead() const
	{
		return canRead;
	}

	bool FileStream::CanWrite() const
	{
		return canWrite;
	}

	int64_t FileStream::Read(void* buffer, size_t size)
	{
		assert(CanRead());

		DWORD bytesRead = -1;
		ReadFile(fileHandle, buffer, static_cast<DWORD>(size), &bytesRead, nullptr);

		position += bytesRead;
		return bytesRead;
	}

	int64_t FileStream::Write(void* buffer, size_t size)
	{
		assert(CanWrite());

		DWORD bytesWritten = -1;
		WriteFile(fileHandle, buffer, static_cast<DWORD>(size), &bytesWritten, nullptr);

		if (position < (GetLength() - static_cast<int64_t>(size)))
			fileSize += bytesWritten;
		position += bytesWritten;

		return bytesWritten;
	}

	void FileStream::OpenRead(const std::wstring& filePath)
	{
		assert(!IsOpen());
		canRead = true;

		fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ, GetShareMode(), NULL, OPEN_EXISTING, GetFileAttribute(), NULL);
		UpdateFileSize();
	}

	void FileStream::OpenWrite(const std::wstring& filePath)
	{
		assert(!IsOpen());
		canWrite = true;

		fileHandle = CreateFileW(filePath.c_str(), GENERIC_WRITE, GetShareMode(), NULL, OPEN_EXISTING, GetFileAttribute(), NULL);
		UpdateFileSize();
	}

	void FileStream::OpenReadWrite(const std::wstring& filePath)
	{
		assert(!IsOpen());
		canRead = canWrite = true;

		fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, GetShareMode(), NULL, OPEN_EXISTING, GetFileAttribute(), NULL);
		UpdateFileSize();
	}

	void FileStream::CreateReadWrite(const std::wstring& filePath)
	{
		assert(!IsOpen());
		canRead = canWrite = true;

		fileHandle = CreateFileW(filePath.c_str(), GENERIC_READ | GENERIC_WRITE, GetShareMode(), NULL, CREATE_ALWAYS, GetFileAttribute(), NULL);
		UpdateFileSize();
	}

	void FileStream::Close()
	{
		if (IsOpen())
		{
			CloseHandle(fileHandle);
			fileHandle = nullptr;
		}
	}

	void FileStream::UpdateFileSize()
	{
		assert(IsOpen());

		LARGE_INTEGER fileSize = {};
		GetFileSizeEx(fileHandle, &fileSize);

		this->fileSize = fileSize.QuadPart;
	}

	unsigned long FileStream::GetShareMode()
	{
		return FILE_SHARE_READ | FILE_SHARE_WRITE;
	}

	unsigned long FileStream::GetFileAttribute()
	{
		return FILE_ATTRIBUTE_NORMAL;
	}
}