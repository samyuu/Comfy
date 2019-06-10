#include "BinaryReader.h"
#include <assert.h>

BinaryReader::BinaryReader()
{
}

BinaryReader::BinaryReader(Stream* stream)
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

int64_t BinaryReader::GetPosition() const
{
	return stream->GetPosition();
}

void* BinaryReader::GetPositionPtr() const
{
	return (void*)stream->GetPosition();
}

void BinaryReader::SetPosition(int64_t position)
{
	return stream->Seek(position);
}

void BinaryReader::SetPosition(void* position)
{
	return stream->Seek((int64_t)position);
}

int64_t BinaryReader::GetLength() const
{
	return stream->GetLength();
}

bool BinaryReader::EndOfFile() const
{
	return stream->EndOfFile();
}

PtrMode BinaryReader::GetPointerMode() const
{
	return pointerMode;
}

void BinaryReader::SetPointerMode(PtrMode mode)
{
	pointerMode = mode;
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

void* BinaryReader::ReadPtr()
{
	switch (pointerMode)
	{
	case PtrMode_32Bit:
		return (void*)ReadInt32();

	case PtrMode_64Bit:
		return (void*)ReadInt64();

	default:
		return nullptr;
	}
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
