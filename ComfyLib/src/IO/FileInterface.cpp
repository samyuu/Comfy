#include "FileInterface.h"
#include "Stream/MemoryStream.h"
#include "Stream/FileStream.h"
#include "BinaryReader.h"
#include "BinaryWriter.h"
#include "Misc/StringHelper.h"

namespace Comfy::IO
{
	void IBinaryReadable::Load(std::string_view filePath)
	{
		MemoryStream stream(filePath);
		BinaryReader reader(stream);
		Read(reader);
	}

	void IBinaryWritable::Save(std::string_view filePath)
	{
		FileStream stream;
		stream.CreateReadWrite(filePath);

		BinaryWriter writer(stream);
		Write(writer);
	}
}
