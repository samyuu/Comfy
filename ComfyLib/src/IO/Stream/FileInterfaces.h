#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::IO
{
	class StreamReader;
	class StreamWriter;

	class IStreamReadable
	{
	public:
		virtual ~IStreamReadable() = default;

		virtual void Read(StreamReader& reader) = 0;
	};

	class IStreamWritable
	{
	public:
		virtual ~IStreamWritable() = default;

		virtual void Write(StreamWriter& writer) = 0;
	};

	// TODO: Get rid of this interface in favor of IStreamReadable, then rename and move file
	class IBufferParsable
	{
	public:
		virtual ~IBufferParsable() = default;

		virtual void Parse(const u8* buffer, size_t bufferSize) = 0;
	};
}
