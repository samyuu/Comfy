#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"

namespace Database
{
	class Database : public FileSystem::IBinaryReadable, public FileSystem::IBinaryWritable
	{
	};
	
	struct DatabaseEntry
	{
	};

	struct DatabaseFileEntry
	{
	};
}