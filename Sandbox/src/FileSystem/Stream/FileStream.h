#pragma once
#include "Stream.h"
#include "Core/CoreTypes.h"

namespace FileSystem
{
	class FileStream : public Stream
	{
	public:
		FileStream();
		FileStream(const WideString& filePath);
		~FileStream();

		virtual void Seek(int64_t position) override;
		virtual int64_t GetPosition() const override;
		virtual int64_t GetLength() const override;

		virtual bool IsOpen() const override;
		virtual bool CanRead() const override;
		virtual bool CanWrite() const override;

		virtual int64_t Read(void* buffer, size_t size) override;
		virtual int64_t Write(const void* buffer, size_t size) override;

		void OpenRead(const WideString& filePath);
		void OpenWrite(const WideString& filePath);
		void OpenReadWrite(const WideString& filePath);
		void CreateWrite(const WideString& filePath);
		void CreateReadWrite(const WideString& filePath);
		virtual void Close() override;

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