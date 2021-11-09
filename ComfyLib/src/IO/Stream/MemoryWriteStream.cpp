#include "MemoryWriteStream.h"

namespace Comfy::IO
{
	MemoryWriteStream::MemoryWriteStream(std::unique_ptr<u8[]>& dataBuffer) : dataBuffer(dataBuffer)
	{
		constexpr size_t reasonableInitialSize = 0x4000;
		dataBuffer = std::make_unique<u8[]>(dataBufferAllocatedSize = reasonableInitialSize);
	}

	void MemoryWriteStream::Seek(FileAddr position)
	{
		dataPosition = Min(static_cast<int64_t>(position), dataLength);
	}

	FileAddr MemoryWriteStream::GetPosition() const
	{
		return static_cast<FileAddr>(dataPosition);
	}

	FileAddr MemoryWriteStream::GetLength() const
	{
		return static_cast<FileAddr>(dataLength);
	}

	bool MemoryWriteStream::IsOpen() const
	{
		return true;
	}

	bool MemoryWriteStream::CanRead() const
	{
		return false;
	}

	bool MemoryWriteStream::CanWrite() const
	{
		return true;
	}

	size_t MemoryWriteStream::ReadBuffer(void* buffer, size_t size)
	{
		return 0;
	}

	size_t MemoryWriteStream::WriteBuffer(const void* buffer, size_t size)
	{
		const auto newDataPosition = dataPosition + size;
		if (newDataPosition > dataBufferAllocatedSize)
		{
			const auto newAllocatedSize = Max(dataBufferAllocatedSize * 2, newDataPosition);

			auto newDataBuffer = std::make_unique<u8[]>(newAllocatedSize);
			std::memcpy(newDataBuffer.get(), dataBuffer.get(), dataLength);

			dataBuffer = std::move(newDataBuffer);
			dataBufferAllocatedSize = newAllocatedSize;
		}

		std::memcpy(dataBuffer.get() + dataPosition, buffer, size);
		dataPosition = newDataPosition;

		if (dataPosition > dataLength)
			dataLength = dataPosition;

		return size;
	}

	void MemoryWriteStream::Close()
	{
	}
}
