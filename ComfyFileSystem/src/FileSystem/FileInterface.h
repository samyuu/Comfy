#pragma once
#include "CoreTypes.h"

namespace FileSystem
{
	class BinaryReader;
	class BinaryWriter;

	class IReadable
	{
	public:
		virtual void Load(const std::string& filePath) = 0;
		virtual void Load(const std::wstring& filePath) = 0;
	};

	class IWritable
	{
	public:
		virtual void Save(const std::string& filePath) = 0;
		virtual void Save(const std::wstring& filePath) = 0;
	};

	class IBinaryReadable : public IReadable
	{
	public:
		virtual void Read(BinaryReader& reader) = 0;

		void Load(const std::string& filePath) override;
		void Load(const std::wstring& filePath) override;
	};

	class IBinaryWritable : public IWritable
	{
	public:
		virtual void Write(BinaryWriter& writer) = 0;

		virtual void Save(const std::string& filePath) override;
		virtual void Save(const std::wstring& filePath) override;
	};

	class IBinaryReadWritable : public IBinaryReadable, public IBinaryWritable
	{
	public:
	};

	class IBufferParsable
	{
	public:
		virtual void Parse(const uint8_t* buffer, size_t bufferSize) = 0;
	};
}