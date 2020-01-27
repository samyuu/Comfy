#pragma once
#include "Stream.h"
#include "Core/CoreTypes.h"

namespace FileSystem
{
	class FileStream : public Stream
	{
	public:
		FileStream();
		FileStream(const std::wstring& filePath);
		~FileStream();

		void Seek(int64_t position) override;
		int64_t GetPosition() const override;
		int64_t GetLength() const override;

		bool IsOpen() const override;
		bool CanRead() const override;
		bool CanWrite() const override;

		int64_t Read(void* buffer, size_t size) override;
		int64_t Write(const void* buffer, size_t size) override;

		void OpenRead(const std::wstring& filePath);
		void OpenWrite(const std::wstring& filePath);
		void OpenReadWrite(const std::wstring& filePath);
		void CreateWrite(const std::wstring& filePath);
		void CreateReadWrite(const std::wstring& filePath);
		void Close() override;

	protected:
		bool canRead = false;
		bool canWrite = false;
		int64_t position = 0L;
		int64_t fileSize = 0L;

		void* fileHandle = nullptr;

		void UpdateFileSize();
		static unsigned long GetShareMode();
		static unsigned long GetFileAttribute();
	};
}