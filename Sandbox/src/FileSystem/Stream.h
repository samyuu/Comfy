#pragma once
#include <stdint.h>

namespace FileSystem
{
	class Stream
	{
	public:
		Stream();
		~Stream();

		bool EndOfFile();
		void Skip(int64_t amount);
		void Rewind();

		virtual void Seek(int64_t position) = 0;
		int64_t RemainingBytes() const;
		virtual int64_t GetPosition() const = 0;
		virtual int64_t GetLength() const = 0;

		virtual bool IsOpen() const = 0;
		virtual bool CanRead() const = 0;
		virtual bool CanWrite() const = 0;

		virtual int64_t Read(void* buffer, size_t size) = 0;
		virtual int64_t Write(void* buffer, size_t size) = 0;

		virtual void Close() = 0;
	};
}
