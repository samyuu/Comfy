#include "Stream.h"

namespace FileSystem
{
	Stream::Stream()
	{
	}

	Stream::~Stream()
	{
	}

	int64_t Stream::RemainingBytes() const
	{
		return GetLength() - GetPosition();
	}

	bool Stream::EndOfFile()
	{
		return GetPosition() >= GetLength();
	}

	void Stream::Skip(int64_t amount)
	{
		Seek(GetPosition() + amount);
	}

	void Stream::Rewind()
	{
		Seek(0L);
	}
}