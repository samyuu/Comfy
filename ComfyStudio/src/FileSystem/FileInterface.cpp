#include "FileInterface.h"
#include "Stream/MemoryStream.h"
#include "Stream/FileStream.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Misc/StringHelper.h"

namespace FileSystem
{
	static void IBinaryReadableLoadBase(IBinaryReadable* readable, Stream* stream)
	{
		BinaryReader reader(stream);
		readable->Read(reader);
		reader.Close();
	}

	static void IBinaryWritableSaveBase(IBinaryWritable* writable, Stream* stream)
	{
		BinaryWriter writer(stream);
		writable->Write(writer);
		writer.Close();
	}

	void IBinaryReadable::Load(const String& filePath)
	{
		MemoryStream stream(filePath);
		IBinaryReadableLoadBase(this, &stream);
		stream.Close();
	}

	void IBinaryReadable::Load(const WideString& filePath)
	{
		MemoryStream stream(filePath);
		IBinaryReadableLoadBase(this, &stream);
		stream.Close();
	}

	void IBinaryWritable::Save(const String& filePath)
	{
		FileStream stream;
		
		WideString widePath = Utf8ToUtf16(filePath);
		stream.CreateReadWrite(widePath);
		IBinaryWritableSaveBase(this, &stream);
		stream.Close();
	}

	void IBinaryWritable::Save(const WideString& filePath)
	{
		FileStream stream;
		stream.CreateReadWrite(filePath);
		IBinaryWritableSaveBase(this, &stream);
		stream.Close();
	}
}