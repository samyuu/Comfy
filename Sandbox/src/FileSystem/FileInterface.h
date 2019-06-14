#pragma once

namespace FileSystem
{
	class BinaryReader;
	class BinaryWriter;

	class IBinaryReadable
	{
	public:
		virtual void Read(BinaryReader& reader) = 0;
	};

	class IBinaryWritable
	{
	public:
		virtual void Read(BinaryWriter& writer) = 0;
	};

	class IBinaryFile : public IBinaryReadable, public IBinaryWritable
	{

	};
}