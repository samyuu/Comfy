#pragma once
#include <string>

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
		virtual void Read(BinaryWriter& writer) = 0;
	};

	class IBinaryFile : public IBinaryReadable, public IBinaryWritable
	{

	};
}