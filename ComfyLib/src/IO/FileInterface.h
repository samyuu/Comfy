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
		virtual void Read(StreamReader& reader) = 0;

		// TODO: Make free standing function
		void Load(std::string_view filePath);
	};

	class IStreamWritable
	{
	public:
		virtual void Write(StreamWriter& writer) = 0;

		// TODO: Make free standing function
		void Save(std::string_view filePath);
	};

	class IBufferParsable
	{
	public:
		virtual void Parse(const u8* buffer, size_t bufferSize) = 0;
	};
}
