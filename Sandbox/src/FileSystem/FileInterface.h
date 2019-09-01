#pragma once
#include "Core/CoreTypes.h"

namespace FileSystem
{
	class BinaryReader;
	class BinaryWriter;

	class IReadable
	{
	public:
		virtual void Load(const String& filePath) = 0;
		virtual void Load(const WideString& filePath) = 0;
	};

	class IWritable
	{
	public:
		virtual void Save(const String& filePath) = 0;
		virtual void Save(const WideString& filePath) = 0;
	};

	class IBinaryReadable : public IReadable
	{
	public:
		virtual void Read(BinaryReader& reader) = 0;

		void Load(const String& filePath) override;
		void Load(const WideString& filePath) override;
	};

	class IBinaryWritable : public IWritable
	{
	public:
		virtual void Write(BinaryWriter& writer) = 0;

		virtual void Save(const String& filePath) override;
		virtual void Save(const WideString& filePath) override;
	};

	class IBinaryFile : public IBinaryReadable, public IBinaryWritable
	{
	public:
	};

	class IBufferParsable
	{
	public:
		virtual void Parse(const uint8_t* buffer) = 0;
	};
}