#pragma once
#include "Types.h"

namespace Comfy::IO
{
	class IStream
	{
	public:
		virtual ~IStream() = default;

	public:
		virtual void Seek(FileAddr position) = 0;
		virtual FileAddr GetPosition() const = 0;
		virtual FileAddr GetLength() const = 0;

		virtual bool IsOpen() const = 0;
		virtual bool CanRead() const = 0;
		virtual bool CanWrite() const = 0;

		virtual size_t ReadBuffer(void* buffer, size_t size) = 0;
		virtual size_t WriteBuffer(const void* buffer, size_t size) = 0;

		virtual void Close() = 0;
	};
}
