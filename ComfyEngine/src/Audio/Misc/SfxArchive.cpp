#include "SfxArchive.h"

namespace Comfy::Audio
{
	SfxArchive::SfxArchive(std::string_view farcPath)
	{
		ParseLoadFArc(farcPath);
	}

	SfxArchive::~SfxArchive()
	{
		if (loadFuture.valid())
			loadFuture.get();

		if (!Audio::AudioEngine::InstanceValid())
			return;

		auto& engine = Audio::AudioEngine::GetInstance();;
		for (auto& loadedSource : loadedSources)
			engine.UnloadSource(loadedSource);
	}

	std::pair<SourceHandle, const Database::SfxEntry*> SfxArchive::FindSource(std::string_view name) const
	{
		if (loadFuture.valid())
			loadFuture.get();

		for (size_t i = 0; i < sfxDB.Entries.size(); i++)
		{
			if (const auto& entry = sfxDB.Entries[i]; entry.Name == name)
				return std::make_pair(loadedSources[i], &entry);
		}

		return std::make_pair(SourceHandle::Invalid, nullptr);

	}

	const std::vector<Database::SfxEntry>& SfxArchive::GetEntries() const
	{
		if (loadFuture.valid())
			loadFuture.get();

		return sfxDB.Entries;
	}

	bool SfxArchive::IsLoaded() const
	{
		return (!loadFuture.valid() || loadFuture._Is_ready());
	}

	void SfxArchive::ParseLoadFArc(std::string_view farcPath)
	{
		loadFuture = std::async(std::launch::async, [this, path = std::string(farcPath)]()
		{
			auto farc = IO::FArc::Open(path);
			if (farc == nullptr)
				return false;

			const auto* sfxDBFArcEntry = farc->FindFile("property.txt");
			if (sfxDBFArcEntry == nullptr)
				return false;

			const auto& farcFiles = farc->GetEntries();
			const auto largestFileSizeInFArc = std::max_element(farcFiles.begin(), farcFiles.end(), [&](auto& a, auto& b)
			{
				return (a.OriginalSize < b.OriginalSize);
			})->OriginalSize;

			auto fileContentBuffer = std::make_unique<u8[]>(largestFileSizeInFArc);
			sfxDBFArcEntry->ReadIntoBuffer(fileContentBuffer.get());
			sfxDB.Parse(fileContentBuffer.get(), sfxDBFArcEntry->OriginalSize);

			auto& engine = Audio::AudioEngine::GetInstance();;

			loadedSources.reserve(sfxDB.Entries.size());
			for (size_t i = 0; i < sfxDB.Entries.size(); i++)
			{
				if (const auto* sfxFile = farc->FindFile(sfxDB.Entries[i].FileName))
				{
					sfxFile->ReadIntoBuffer(fileContentBuffer.get());
					loadedSources.push_back(engine.LoadAudioSource(sfxFile->Name, fileContentBuffer.get(), sfxFile->OriginalSize));
				}
				else
				{
					loadedSources.push_back(SourceHandle::Invalid);
				}
			}

			return true;
		});
	}
}
