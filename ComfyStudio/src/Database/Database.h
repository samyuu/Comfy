#pragma once
#include "Types.h"
#include "FileSystem/FileInterface.h"

namespace Database
{
	class BinaryDatabase : public FileSystem::IBinaryReadable, public FileSystem::IBinaryWritable
	{
	public:
		struct Entry
		{
		};

		struct FileEntry
		{
		};
	};
}
