#include "FileInterface.h"
#include "MemoryStream.h"
#include "BinaryReader.h"

namespace FileSystem
{
	static void IBinaryReadableLoadBase(IBinaryReadable* readable, Stream* stream)
	{
		BinaryReader reader(stream);
		readable->Read(reader);
		reader.Close();
	}

	void IBinaryReadable::Load(const std::string& filePath)
	{
		MemoryStream stream(filePath);
		IBinaryReadableLoadBase(this, &stream);
		stream.Close();
	}

	void IBinaryReadable::Load(const std::wstring& filePath)
	{
		MemoryStream stream(filePath);
		IBinaryReadableLoadBase(this, &stream);
		stream.Close();
	}
}