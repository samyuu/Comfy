#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Audio/Audio.h"
#include "Database/Game/GmBtnSfxDB.h"

namespace Comfy::Studio::Editor
{
	class SoundEffectManager : NonCopyable
	{
	public:
		SoundEffectManager();
		~SoundEffectManager() = default;

	public:
		Audio::SourceHandle GetButtonSound(u32 id) const;
		Audio::SourceHandle GetSlideSound(u32 id) const;
		std::array<Audio::SourceHandle, 4> GetChainSlideSound(u32 id) const;
		std::array<Audio::SourceHandle, 32> GetSliderTouchSound(u32 id) const;

		Audio::SourceHandle Find(std::string_view name) const;

		const Database::GmBtnSfxDB& ViewBtnSfxDB(Database::GmBtnSfxType type) const;

		bool IsAsyncLoaded() const;
		void WaitUntilAsyncLoaded();

	private:
		void InitializeLoadFiles();
		void InitializeRegisterSfxNameMap();
		void InitializeButtonIDLookupTables();

	private:
		mutable std::future<void> loadFuture;

		std::vector<std::unique_ptr<Audio::SfxArchive>> sfxArchives;

		struct ButtonIDLookupData
		{
			// NOTE: Map ID as index to sound source
			std::vector<Audio::SourceHandle> Button;
			std::vector<Audio::SourceHandle> Slide;
			std::vector<std::array<Audio::SourceHandle, 4>> ChainSlide;
			std::vector<std::array<Audio::SourceHandle, 32>> SliderTouch;
		} buttonIDLookup;

		std::array<Database::GmBtnSfxDB, EnumCount<Database::GmBtnSfxType>()> btnSfxDBs;

		// NOTE: Assumes all archives and entries are read only and stable pointers. The string_view keys are also owned by the sfx entries
		std::unordered_map<std::string_view, std::pair<Audio::SourceHandle, const Database::SfxEntry*>> sfxNameMap;
	};
}
