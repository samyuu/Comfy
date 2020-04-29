#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::IO
{
	class StreamReader;
	class StreamWriter;

	class IReadable
	{
	public:
		virtual void Load(std::string_view filePath) = 0;
	};

	class IWritable
	{
	public:
		virtual void Save(std::string_view filePath) = 0;
	};

	class IBinaryReadable : public IReadable
	{
	public:
		virtual void Read(StreamReader& reader) = 0;
		void Load(std::string_view filePath) override;
	};

	class IBinaryWritable : public IWritable
	{
	public:
		virtual void Write(StreamWriter& writer) = 0;
		void Save(std::string_view filePath) override;
	};

	class IBinaryReadWritable : public IBinaryReadable, public IBinaryWritable
	{
	public:
	};

	class IBufferParsable
	{
	public:
		virtual void Parse(const u8* buffer, size_t bufferSize) = 0;
	};
}
