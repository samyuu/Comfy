#pragma once
#include "Database.h"

namespace Comfy::Database
{
	struct AetSceneEntry : BinaryDatabase::Entry
	{
		AetSceneID ID;
		std::string Name;
		i16 Index;
	};

	struct AetSetEntry : BinaryDatabase::Entry, BinaryDatabase::FileEntry
	{
		AetSetID ID;
		std::string Name;
		SprSetID SprSetID;
		std::vector<AetSceneEntry> SceneEntries;
		std::string FileName;

		AetSceneEntry* GetSceneEntry(std::string_view name);
	};

	class AetDB final : public BinaryDatabase
	{
	public:
		std::vector<AetSetEntry> Entries;

	public:
		AetSetEntry* GetAetSetEntry(std::string_view name);

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
	};
}
