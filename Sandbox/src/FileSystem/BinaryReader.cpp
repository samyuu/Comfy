#include "BinaryReader.h"
#include <assert.h>

namespace FileSystem
{
	BinaryReader::BinaryReader()
	{
		SetPointerMode(PtrMode::Mode32Bit);
		SetEndianness(Endianness::Little);
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

	void BinaryReader::SetPointerMode(PtrMode value)
	{
		pointerMode = value;

		switch (pointerMode)
		{
		case PtrMode::Mode32Bit:
			readPtrFunction = Read32BitPtr; 
			return;

		case PtrMode::Mode64Bit:
			readPtrFunction = Read64BitPtr;
			return;

		default:
			break;
		}

		assert(false);
	}

	Endianness BinaryReader::GetEndianness() const
	{
		return endianness;
	}

	void BinaryReader::SetEndianness(Endianness value)
	{
		endianness = value;
		
		switch (value)
		{
		case Endianness::Little:
			readInt16Function = LE_ReadInt16;
			readUInt16Function = LE_ReadUInt16;
			readInt32Function = LE_ReadInt32;
			readUInt32Function = LE_ReadUInt32;
			readInt64Function = LE_ReadInt64;
			readUInt64Function = LE_ReadUInt64;
			readFloatFunction = LE_ReadFloat;
			readDoubleFunction = LE_ReadDouble;
			return;
		
		case Endianness::Big:
			readInt16Function = BE_ReadInt16;
			readUInt16Function = BE_ReadUInt16;
			readInt32Function = BE_ReadInt32;
			readUInt32Function = BE_ReadUInt32;
			readInt64Function = BE_ReadInt64;
			readUInt64Function = BE_ReadUInt64;
			readFloatFunction = BE_ReadFloat;
			readDoubleFunction = BE_ReadDouble;
			return;
		
		default:
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