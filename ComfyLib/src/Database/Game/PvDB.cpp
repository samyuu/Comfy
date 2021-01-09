#include "PvDB.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"
#include "Misc/TextDatabaseParser.h"
#include "IO/Stream/Manipulator/StreamWriter.h"

namespace Comfy::Database
{
	void PvDB::Parse(const u8* buffer, size_t bufferSize)
	{
		// TODO:
	}

	IO::StreamResult PvDB::Write(IO::StreamWriter& writer)
	{
		// TODO:
		return IO::StreamResult::UnknownError;
	}
}
