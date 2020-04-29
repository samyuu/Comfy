#include "FileInterface.h"
#include "Stream/MemoryStream.h"
#include "Stream/FileStream.h"
#include "StreamReader.h"
#include "StreamWriter.h"
#include "Misc/StringHelper.h"

namespace Comfy::IO
{
	void IBinaryReadable::Load(std::string_view filePath)
	{
		MemoryStream stream(filePath);
		StreamReader reader(stream);
		Read(reader);
	}

	void IBinaryWritable::Save(std::string_view filePath)
	{
		FileStream stream;
		stream.CreateReadWrite(filePath);

		StreamWriter writer(stream);
		Write(writer);
	}
}
