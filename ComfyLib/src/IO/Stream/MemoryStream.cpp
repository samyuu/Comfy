#include "MemoryStream.h"
#include "FileStream.h"
#include "Misc/StringHelper.h"
#include <assert.h>
#include <algorithm>

namespace Comfy::IO
{
	MemoryStream::MemoryStream()
	{
		dataSource = &dataVector;
	}

	MemoryStream::MemoryStream(std::string_view filePath) : MemoryStream()
	{
		FromFile(filePath);
	}

	MemoryStream::MemoryStream(IStream& stream) : MemoryStream()
	{
		FromStream(stream);
	}

	MemoryStream::MemoryStream(std::vector<u8>& source)
	{
		FromStreamSource(source);
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
		return true;
	}

	bool MemoryStream::CanRead() const
	{
		return canRead;
	}

	bool MemoryStream::CanWrite() const
	{
		return false;
	}

	size_t MemoryStream::ReadBuffer(void* buffer, size_t size)
	{
		assert(canRead);
		int64_t bytesRead = std::min(static_cast<int64_t>(size), static_cast<int64_t>(RemainingBytes()));

		void* source = &(*dataSource)[static_cast<size_t>(position)];
		memcpy(buffer, source, bytesRead);

		position += static_cast<FileAddr>(bytesRead);
		return bytesRead;
	}

	size_t MemoryStream::WriteBuffer(const void* buffer, size_t size)
	{
		assert(canRead);
		dataSource->resize(dataSource->size() + size);

		const u8* bufferStart = reinterpret_cast<const u8*>(buffer);
		const u8* bufferEnd = &bufferStart[size];
		std::copy(bufferStart, bufferEnd, std::back_inserter(*dataSource));

		return size;
	}

	void MemoryStream::FromStreamSource(std::vector<u8>& source)
	{
		dataSource = &source;

		canRead = true;
		dataSize = static_cast<FileAddr>(source.size());
	}

	void MemoryStream::FromFile(std::string_view filePath)
	{
		FileStream fileStream(filePath);
		FromStream(fileStream);
	}

	void MemoryStream::FromStream(IStream& stream)
	{
		assert(stream.CanRead());

		canRead = true;
		dataSize = stream.RemainingBytes();
		dataSource->resize(static_cast<size_t>(dataSize));

		const auto prePos = stream.GetPosition();
		stream.ReadBuffer(dataSource->data(), static_cast<size_t>(dataSize));
		stream.Seek(prePos);
	}

	void MemoryStream::Close()
	{
	}
}
