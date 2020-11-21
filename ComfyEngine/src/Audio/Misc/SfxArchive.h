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
		std::pair<SourceHandle, const Database::SfxEntry*> FindSource(std::string_view name) const;
		const std::vector<Database::SfxEntry>& GetEntries() const;

		bool IsLoaded() const;

	private:
		void ParseLoadFArc(std::string_view farcPath);

	private:
		mutable std::future<bool> loadFuture;

		Database::SfxDB sfxDB = {};
		// NOTE: Mapped to the DB source entries by the same index
		std::vector<SourceHandle> loadedSources;
	};
}
