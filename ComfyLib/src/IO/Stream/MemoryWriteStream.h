#pragma once
#include "IStream.h"
#include "CoreTypes.h"

namespace Comfy::IO
{
	// TEMP: Eventually this should be rewritten more cleanly as part of the normal MemoryStream
	class MemoryWriteStream : public IStream, NonCopyable
	{
	public:
		MemoryWriteStream(std::unique_ptr<u8[]>& dataBuffer);
		~MemoryWriteStream() = default;

	public:
		void Seek(FileAddr position) override;
		FileAddr GetPosition() const override;
		FileAddr GetLength() const override;

		bool IsOpen() const override;
		bool CanRead() const override;
		bool CanWrite() const override;

		size_t ReadBuffer(void* buffer, size_t size) override;
		size_t WriteBuffer(const void* buffer, size_t size) override;

		void Close() override;

	protected:
		std::unique_ptr<u8[]>& dataBuffer;
		size_t dataBufferAllocatedSize = 0;

		int64_t dataPosition = 0, dataLength = 0;
	};
}
