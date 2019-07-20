#include "BinaryReader.h"
#include <assert.h>

namespace FileSystem
{
	BinaryReader::BinaryReader()
	{
		SetPointerMode(PtrMode_32Bit);
	}

	BinaryReader::BinaryReader(Stream* stream) : BinaryReader()
	{
		OpenStream(stream);
	}

	BinaryReader::~BinaryReader()
	{
		Close();
	}

	void BinaryReader::OpenStream(Stream* stream)
	{
		assert(!IsOpen());
		assert(stream->CanRead());

		this->stream = stream;
	}

	void BinaryReader::Close()
	{
		if (IsOpen() && !GetLeaveOpen())
			stream->Close();
	}

	bool BinaryReader::IsOpen()
	{
		return stream != nullptr;
	}

	bool BinaryReader::GetLeaveOpen()
	{
		return leaveOpen;
	}

	PtrMode BinaryReader::GetPointerMode() const
	{
		return pointerMode;
	}

	void BinaryReader::SetPointerMode(PtrMode mode)
	{
		pointerMode = mode;

		switch (pointerMode)
		{
		case PtrMode_32Bit:
			readPtrFunction = Read32BitPtr; 
			return;

		case PtrMode_64Bit:
			readPtrFunction = Read64BitPtr;
			return;

		default:
			readPtrFunction = nullptr;
			break;
		}

		assert(false);
	}

	int64_t BinaryReader::Read(void* buffer, size_t size)
	{
		return stream->Read(buffer, size);
	}

	void BinaryReader::ReadAt(void* position, const std::function<void(BinaryReader&)>& func)
	{
		int64_t prePos = GetPosition();
		SetPosition(position);
		func(*this);
		SetPosition(prePos);
	}

	std::string BinaryReader::ReadStr()
	{
		// Account for the ending zero byte
		size_t length = 1;

		int64_t prePos = GetPosition();
		{
			while (ReadChar() != '\0' && !EndOfFile())
				length++;
		}
		SetPosition(prePos);

		if (length == 1)
			return "";

		char* buffer = (char*)alloca(length * sizeof(char));
		Read(buffer, length * sizeof(char));

		buffer[length - 1] = '\0';

		auto value = std::string(buffer);
		return value;
	}

	std::string BinaryReader::ReadStrPtr()
	{
		void* stringPointer = ReadPtr();
		if (stringPointer == nullptr)
			return "";

		return ReadAt<std::string>(stringPointer, [&](BinaryReader&)
		{
			return ReadStr();
		});
	}
}