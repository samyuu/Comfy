#include "MemoryStream.h"
#include "FileStream.h"
#include <assert.h>
#include <stdlib.h>
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
		this->position = __min(position, GetLength());
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
		return data != nullptr;
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

		auto remaining = RemainingBytes();
		int64_t bytesRead = __min(size, remaining);

		void* source = &data[position];
		memcpy(buffer, source, bytesRead);

		position += bytesRead;
		return bytesRead;
	}

	int64_t MemoryStream::Write(void* buffer, size_t size)
	{
		assert(CanWrite());

		// TODO: how should this work?
		return 0;
	}

	void MemoryStream::FromFile(const std::string& filePath)
	{
		FromFile(std::wstring(filePath.begin(), filePath.end()));
	}

	void MemoryStream::FromFile(const std::wstring& filePath)
	{
		FileStream fileStream(filePath);
		FromStream(&fileStream);
		fileStream.Close();
	}

	void MemoryStream::FromStream(Stream* stream)
	{
		assert(!IsOpen());
		assert(stream->CanRead());

		canRead = true;
		dataSize = stream->RemainingBytes();
		data = new uint8_t[dataSize];

		int64_t prePos = stream->GetPosition();
		stream->Read(data, dataSize);
		stream->Seek(prePos);
	}

	void MemoryStream::Close()
	{
		if (IsOpen())
		{
			delete[] data;
			data = nullptr;
		}
	}
}