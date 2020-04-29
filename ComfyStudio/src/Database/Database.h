#pragma once
#include "Types.h"
#include "Resource/IDTypes.h"
#include "IO/FileInterface.h"

namespace Comfy::Database
{
	class BinaryDatabase : public IO::IBinaryReadable, public IO::IBinaryWritable
	{
	public:
		struct Entry
		{
		};

		struct FileEntry
		{
		};
	};

	// TODO:
	class TextDatabase : public IO::IBufferParsable
	{
	public:
		TextDatabase() = delete;

		// virtual std::string_view GetFileName() const = 0;
	};

	// TODO:
	class ObjDB final : public BinaryDatabase {};

	class Auth3DDB final : public TextDatabase {};

	class ChrItemDB final : public TextDatabase {};
	class ModuleDB final : public TextDatabase {};
	class CstmItemDB final : public TextDatabase {};

	class PvDB final : public TextDatabase {};
	class FontMapDB final : public TextDatabase {};
}
