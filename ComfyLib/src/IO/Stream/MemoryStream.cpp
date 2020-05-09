#include "MemoryStream.h"
#include "FileStream.h"
#include "Misc/StringHelper.h"
#include <algorithm>

namespace Comfy::IO
{
	MemoryStream::MemoryStream()
	{
		dataVectorPtr = &owningDataVector;
	}

	MemoryStream::MemoryStream(MemoryStream&& other) : MemoryStream()
	{
		if (other.IsOwning())
			owningDataVector = std::move(other.owningDataVector);
		else
			dataVectorPtr = other.dataVectorPtr;

		position = other.position;
		dataSize = other.dataSize;

		other.Close();
	}

	MemoryStream::~MemoryStream()
	{
		Close();
	}

	void MemoryStream::Seek(FileAddr position)
	{
		this->position = position;
		this->position = std::min(position, GetLength());
	}

	FileAddr MemoryStream::GetPosition() const
	{
		return position;
	}

	FileAddr MemoryStream::GetLength() const
	{
		return dataSize;
	}

	bool MemoryStream::IsOpen() const
	{
		return (dataVectorPtr != nullptr);
	}

	bool MemoryStream::CanRead() const
	{
		return (dataVectorPtr != nullptr);
	}

	bool MemoryStream::CanWrite() const
	{
		return false;
	}

	bool MemoryStream::IsOwning() const
	{
		return (dataVectorPtr == &owningDataVector);
	}

	size_t MemoryStream::ReadBuffer(void* buffer, size_t size)
	{
		assert(dataVectorPtr != nullptr);

		const auto remainingSize = (GetLength() - GetPosition());
		const i64 bytesRead = std::min(static_cast<i64>(size), static_cast<i64>(remainingSize));

		void* source = &(*dataVectorPtr)[static_cast<size_t>(position)];
		std::memcpy(buffer, source, bytesRead);

		position += static_cast<FileAddr>(bytesRead);
		return bytesRead;
	}

	size_t MemoryStream::WriteBuffer(const void* buffer, size_t size)
	{
		assert(dataVectorPtr != nullptr);
		dataVectorPtr->resize(dataVectorPtr->size() + size);

		const u8* bufferStart = reinterpret_cast<const u8*>(buffer);
		const u8* bufferEnd = &bufferStart[size];
		std::copy(bufferStart, bufferEnd, std::back_inserter(*dataVectorPtr));

		return size;
	}

	void MemoryStream::FromStreamSource(std::vector<u8>& source)
	{
		dataVectorPtr = &source;
		dataSize = static_cast<FileAddr>(source.size());
	}

	void MemoryStream::FromStream(IStream& stream)
	{
		assert(stream.CanRead());
		dataSize = stream.GetLength() - stream.GetPosition();
		dataVectorPtr->resize(static_cast<size_t>(dataSize));

		const auto prePos = stream.GetPosition();
		stream.ReadBuffer(dataVectorPtr->data(), static_cast<size_t>(dataSize));
		stream.Seek(prePos);
	}

	void MemoryStream::Close()
	{
		position = {};
		dataSize = {};
		dataVectorPtr = nullptr;
		owningDataVector.clear();
	}
}
