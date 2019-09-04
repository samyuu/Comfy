#include "FileArchive.h"

namespace FileSystem
{
	ArchiveEntry::ArchiveEntry(FileArchive* parent) : parent(parent)
	{
		assert(parent != nullptr);
	}

	Vector<uint8_t> ArchiveEntry::ReadVector() const
	{
		return parent->ReadArchiveEntryIntoVector(*this);
	}

	void ArchiveEntry::Read(void* fileContentOut) const
	{
		parent->ReadArchiveEntry(*this, fileContentOut);
	}

	ConstArchiveEntryIterator FileArchive::begin() const
	{
		return archiveEntries.begin();
	}

	ConstArchiveEntryIterator FileArchive::end() const
	{
		return archiveEntries.end();
	}

	size_t FileArchive::size() const
	{
		return archiveEntries.size();
	}

	const ArchiveEntry* FileArchive::GetFile(const String& name) const
	{
		for (const ArchiveEntry& entry : archiveEntries)
		{
			// NOTE: Should this be case insensitive? Probably not.
			if (entry.Name == name)
				return &entry;
		}

		return nullptr;
	}

	Vector<uint8_t> FileArchive::ReadArchiveEntryIntoVector(const ArchiveEntry& entry)
	{
		Vector<uint8_t> fileData(entry.FileSize);
		ReadArchiveEntry(entry, fileData.data());
		return fileData;
	}
}
