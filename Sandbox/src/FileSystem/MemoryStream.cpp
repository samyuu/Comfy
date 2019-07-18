#include "MemoryStream.h"
#include "FileStream.h"
#include <assert.h>
#include <algorithm>
#include <memory>

namespace FileSystem
{
	MemoryStream::MemoryStream()
	{
	}

	MemoryStream::MemoryStream(const std::string& filePath)
	{
		FromFile(filePath);
	}

	MemoryStream::MemoryStream(const std::wstring& filePath)
	{
		FromFile(filePath);
	}

	MemoryStream::MemoryStream(Stream* stream)
	{
		FromStream(stream);
	}

	MemoryStream::~MemoryStream()
	{
	}

	void MemoryStream::Seek(int64_t position)
	{
		this->position = position;
		this->position = std::min(position, GetLength());
	}

	int64_t MemoryStream::GetPosition() const
	{
		return position;
	}

	int64_t MemoryStream::GetLength() const
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

	int64_t MemoryStream::Read(void* buffer, size_t size)
	{
		assert(CanRead());

		int64_t remaining = RemainingBytes();
		int64_t bytesRead = std::min(static_cast<int64_t>(size), remaining);

		void* source = &data[position];
		memcpy(buffer, source, bytesRead);

		position += bytesRead;
		return bytesRead;
	}

	int64_t MemoryStream::Write(const void* buffer, size_t size)
	{
		assert(CanWrite());

		data.resize(data.size() + size);

		const uint8_t* bufferStart = reinterpret_cast<const uint8_t*>(buffer);
		const uint8_t* bufferEnd = &bufferStart[size];
		std::copy(bufferStart, bufferEnd, std::back_inserter(data));

		return size;
	}

	void MemoryStream::FromFile(const std::string& filePath)
	{
		auto widePath = std::wstring(filePath.begin(), filePath.end());
		FromFile(widePath);
	}

	void MemoryStream::FromFile(const std::wstring& filePath)
	{
		FileStream fileStream(filePath);
		FromStream(&fileStream);
		fileStream.Close();
	}

	void MemoryStream::FromStream(Stream* stream)
	{
		assert(stream->CanRead());

		canRead = true;
		dataSize = stream->RemainingBytes();
		data.resize(dataSize);

		int64_t prePos = stream->GetPosition();
		stream->Read(data.data(), dataSize);
		stream->Seek(prePos);
	}

	void MemoryStream::Close()
	{
	}
}