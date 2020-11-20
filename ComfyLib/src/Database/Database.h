#pragma once
#include "Types.h"
#include "Resource/IDTypes.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Database
{
	struct DateEntry { i32 Day, Month, Year; };

	class BinaryDatabase : public IO::IStreamReadable, public IO::IStreamWritable
	{
	public:
		struct Entry
		{
		};

		struct FileEntry
		{
		};
	};

	class TextDatabase : public IO::IBufferParsable
	{
	public:
	};

	// TODO:
	class StrDB final : public BinaryDatabase {};
	class ObjDB final : public BinaryDatabase {};
	class BoneDB final : public BinaryDatabase {};
	class StageDB final : public BinaryDatabase {};
	class Auth3DDB final : public TextDatabase {};
	class GmChrItemDB final : public TextDatabase {};
	class GmCstmItemDB final : public TextDatabase {};
	class GmModuleDB final : public TextDatabase {};
	class GmPlateDB final : public TextDatabase {};
	class GmPvListDB final : public TextDatabase {};
	class HandItemDB final : public TextDatabase {};
	class PvDB final : public TextDatabase {};
}
