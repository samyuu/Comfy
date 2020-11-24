#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Audio/Core/AudioEngine.h"
#include "Database/SfxDB.h"
#include "IO/Archive/FArc.h"

namespace Comfy::Audio
{
	class SfxArchive : NonCopyable
	{
	public:
		SfxArchive(std::string_view farcPath);
		~SfxArchive();

	public:
		std::pair<SourceHandle, const Database::SfxEntry*> Find(std::string_view name) const;
		SourceHandle GetSource(const Database::SfxEntry& entry) const;

		const std::vector<Database::SfxEntry>& GetEntries() const;

		bool IsAsyncLoaded() const;
		void WaitUntilAsyncLoaded();

	private:
		void ParseLoadFArc(std::string_view farcPath);

	private:
		mutable std::future<bool> loadFuture;

		Database::SfxDB sfxDB = {};
		// NOTE: Mapped to the DB source entries by the same index
		std::vector<SourceHandle> loadedSources;
	};
}
