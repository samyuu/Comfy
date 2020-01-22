#pragma once
#include "Database.h"

namespace Database
{
	struct TxpEntry : BinaryDatabase::Entry
	{
		TxpID ID;
		std::string Name;
	};

	class TxpDB final : public BinaryDatabase
	{
	public:
		std::vector<TxpEntry> Entries;

		void Read(FileSystem::BinaryReader& reader) override;
		void Write(FileSystem::BinaryWriter& writer) override;

	private:
	};
}
