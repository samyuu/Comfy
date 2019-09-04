#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"

namespace FileSystem
{
	class FileArchive;

	class ArchiveEntry
	{
	public:
		ArchiveEntry(FileArchive* parent);

	public:
		String Name;
		uint64_t FileOffset;
		uint64_t CompressedSize;
		uint64_t FileSize;

		Vector<uint8_t> ReadVector() const;
		void Read(void* fileContentOut) const;

	private:
		FileArchive* parent;
	};

	using ArchiveEntryIterator = Vector<ArchiveEntry>::iterator;
	using ConstArchiveEntryIterator = Vector<ArchiveEntry>::const_iterator;

	class FileArchive
	{
		friend class ArchiveEntry;
	
	public:
		ConstArchiveEntryIterator begin() const;
		ConstArchiveEntryIterator end() const;
		size_t size() const;

		const ArchiveEntry* GetFile(const String& name) const;

	protected:
		Vector<ArchiveEntry> archiveEntries;

		Vector<uint8_t> ReadArchiveEntryIntoVector(const ArchiveEntry& entry);
		virtual void ReadArchiveEntry(const ArchiveEntry& entry, void* fileContentOut) = 0;
	};
}