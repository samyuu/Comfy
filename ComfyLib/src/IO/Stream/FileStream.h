#pragma once
#include "Types.h"
#include "IStream.h"

namespace Comfy::IO
{
	class FileStream final : public IStream, NonCopyable
	{
	public:
		FileStream() = default;
		FileStream(FileStream&& other);
		~FileStream();

	public:
		void Seek(FileAddr position) override;
		FileAddr GetPosition() const override;
		FileAddr GetLength() const override;

		bool IsOpen() const override;
		bool CanRead() const override;
		bool CanWrite() const override;

		size_t ReadBuffer(void* buffer, size_t size) override;
		size_t WriteBuffer(const void* buffer, size_t size) override;

		void OpenRead(std::string_view filePath);
		void OpenWrite(std::string_view filePath);
		void OpenReadWrite(std::string_view filePath);
		void CreateWrite(std::string_view filePath);
		void CreateReadWrite(std::string_view filePath);
		void Close() override;

	protected:
		void UpdateFileSize();

	protected:
		bool canRead = false;
		bool canWrite = false;
		FileAddr position = {};
		FileAddr fileSize = {};

		void* fileHandle = nullptr;
	};
}
