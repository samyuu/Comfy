#include "SoundEffectManager.h"
#include "IO/File.h"

namespace Comfy::Studio::Editor
{
	SoundEffectManager::SoundEffectManager()
	{
		loadFuture = std::async(std::launch::async, [this]()
		{
			InitializeLoadFiles();
			InitializeRegisterSfxNameMap();
			InitializeButtonIDLookupTables();
		});
	}

	SoundEffectManager::~SoundEffectManager()
	{
		if (loadFuture.valid())
			loadFuture.get();

		for (const auto& sfxArchive : sfxArchives)
		{
			if (sfxArchive != nullptr)
				sfxArchive->WaitUntilAsyncLoaded();
		}
	}

	Audio::SourceHandle SoundEffectManager::GetButtonSound(u32 id) const
	{
		if (loadFuture.valid()) loadFuture.get();
		return IndexOr(id, buttonIDLookup.Button, Audio::SourceHandle::Invalid);
	}

	Audio::SourceHandle SoundEffectManager::GetSlideSound(u32 id) const
	{
		if (loadFuture.valid()) loadFuture.get();
		return IndexOr(id, buttonIDLookup.Slide, Audio::SourceHandle::Invalid);
	}

	std::array<Audio::SourceHandle, 4> SoundEffectManager::GetChainSlideSound(u32 id) const
	{
		if (loadFuture.valid()) loadFuture.get();
		if (auto* found = IndexOrNull(id, buttonIDLookup.ChainSlide))
			return *found;

		std::array<Audio::SourceHandle, 4> result;
		std::fill(result.begin(), result.end(), Audio::SourceHandle::Invalid);
		return result;
	}

	std::array<Audio::SourceHandle, 32> SoundEffectManager::GetSliderTouchSound(u32 id) const
	{
		if (loadFuture.valid()) loadFuture.get();
		if (auto* found = IndexOrNull(id, buttonIDLookup.SliderTouch))
			return *found;

		std::array<Audio::SourceHandle, 32> result;
		std::fill(result.begin(), result.end(), Audio::SourceHandle::Invalid);
		return result;
	}

	Audio::SourceHandle SoundEffectManager::Find(std::string_view name) const
	{
		if (loadFuture.valid()) loadFuture.get();
		const auto found = sfxNameMap.find(name);
		return (found != sfxNameMap.end()) ? found->second.first : Audio::SourceHandle::Invalid;
	}

	const std::vector<const Database::GmBtnSfxEntry*>& SoundEffectManager::ViewSortedBtnSfxDB(Database::GmBtnSfxType type) const
	{
		if (loadFuture.valid()) loadFuture.get();
		return sortedBtnSfxDBs[static_cast<u8>(type)];
	}

	bool SoundEffectManager::IsAsyncLoaded() const
	{
		return (!loadFuture.valid() || loadFuture._Is_ready());
	}

	void SoundEffectManager::WaitUntilAsyncLoaded()
	{
		if (loadFuture.valid())
			loadFuture.get();
	}

	void SoundEffectManager::InitializeLoadFiles()
	{
		static constexpr std::array sfxArchivePaths =
		{
			"dev_rom/sound/button.farc",
			"dev_rom/sound/slide_se.farc",
			"dev_rom/sound/slide_long.farc",
			"dev_rom/sound/slide_windchime.farc",
			"dev_rom/sound/slide_laser.farc",
			"dev_rom/sound/se_ft.farc",
		};

		assert(sfxArchives.empty());
		sfxArchives.reserve(sfxArchivePaths.size());

		for (const char* path : sfxArchivePaths)
			sfxArchives.push_back(std::make_unique<Audio::SfxArchive>(path));

		auto readPaseBtnSfxDB = [this](Database::GmBtnSfxType type, std::string_view filePath)
		{
			auto& outDB = btnSfxDBs[static_cast<u8>(type)];
			auto& outSorted = sortedBtnSfxDBs[static_cast<u8>(type)];
			assert(outDB.Entries.empty() && outSorted.empty());

			if (const auto[fileContent, fileSize] = IO::File::ReadAllBytes(filePath); fileContent != nullptr && fileSize > 0)
			{
				outDB.Parse(fileContent.get(), fileSize);

				outSorted.reserve(outDB.Entries.size());
				for (const auto& entry : outDB.Entries)
					outSorted.push_back(&entry);
				std::sort(outSorted.begin(), outSorted.end(), [](auto* a, auto* b) { return (a->SortIndex < b->SortIndex); });
			}
		};

		readPaseBtnSfxDB(Database::GmBtnSfxType::Button, "dev_rom/gm_btn_se_tbl.farc<gm_btn_se_id.bin>");
		readPaseBtnSfxDB(Database::GmBtnSfxType::Slide, "dev_rom/gm_slide_se_tbl.farc<gm_slide_se_id.bin>");
		readPaseBtnSfxDB(Database::GmBtnSfxType::ChainSlide, "dev_rom/gm_chainslide_se_tbl.farc<gm_chainslide_se_id.bin>");
		readPaseBtnSfxDB(Database::GmBtnSfxType::SliderTouch, "dev_rom/gm_slidertouch_se_tbl.farc<gm_slidertouch_se_id.bin>");
	}

	void SoundEffectManager::InitializeRegisterSfxNameMap()
	{
		assert(sfxNameMap.empty());
		for (const auto& archive : sfxArchives)
		{
			archive->WaitUntilAsyncLoaded();
			sfxNameMap.reserve(sfxNameMap.size() + archive->GetEntries().size());

			for (const auto& entry : archive->GetEntries())
				sfxNameMap[entry.Name] = std::pair(archive->GetSource(entry), &entry);
		}
	}

	void SoundEffectManager::InitializeButtonIDLookupTables()
	{
		// NOTE: Can't call public methods here because of the async load future

		auto findIDCount = [this](Database::GmBtnSfxType type)
		{
			const auto& dbEntries = btnSfxDBs[static_cast<u8>(type)].Entries;
			return dbEntries.empty() ? 0 : std::max_element(dbEntries.begin(), dbEntries.end(), [](auto& a, auto& b) { return (a.ID < b.ID); })->ID + 1;
		};

		buttonIDLookup.Button.resize(findIDCount(Database::GmBtnSfxType::Button));
		buttonIDLookup.Slide.resize(findIDCount(Database::GmBtnSfxType::Slide));
		buttonIDLookup.ChainSlide.resize(findIDCount(Database::GmBtnSfxType::ChainSlide));
		buttonIDLookup.SliderTouch.resize(findIDCount(Database::GmBtnSfxType::SliderTouch));

		auto tryFindByID = [this](Database::GmBtnSfxType type, u32 id) -> const Database::GmBtnSfxEntry*
		{
			const auto& db = btnSfxDBs[static_cast<u8>(type)];
			if (InBounds(id, db.Entries) && db.Entries[id].ID == id)
				return &db.Entries[id];
			else
				return FindIfOrNull(db.Entries, [id](const auto& e) { return (e.ID == id); });
		};

		auto findSfx = [&](std::string_view name)
		{
			const auto found = sfxNameMap.find(name);
			return (found != sfxNameMap.end()) ? found->second.first : Audio::SourceHandle::Invalid;
		};

		for (u32 id = 0; id < buttonIDLookup.Button.size(); id++)
		{
			if (auto* entry = tryFindByID(Database::GmBtnSfxType::Button, (id == 0) ? 1 : id))
				buttonIDLookup.Button[id] = findSfx(entry->SfxName);
			else
				buttonIDLookup.Button[id] = Audio::SourceHandle::Invalid;
		}

		for (u32 id = 0; id < buttonIDLookup.Slide.size(); id++)
		{
			if (auto* entry = tryFindByID(Database::GmBtnSfxType::Slide, id))
				buttonIDLookup.Slide[id] = findSfx(entry->SfxName);
			else
				buttonIDLookup.Slide[id] = Audio::SourceHandle::Invalid;
		}

		for (u32 id = 0; id < buttonIDLookup.ChainSlide.size(); id++)
		{
			if (auto* entry = tryFindByID(Database::GmBtnSfxType::ChainSlide, id))
			{
				buttonIDLookup.ChainSlide[id][0] = findSfx(entry->Chain.SfxNameFirst);
				buttonIDLookup.ChainSlide[id][1] = findSfx(entry->Chain.SfxNameSub);
				buttonIDLookup.ChainSlide[id][2] = findSfx(entry->Chain.SfxNameSuccess);
				buttonIDLookup.ChainSlide[id][3] = findSfx(entry->Chain.SfxNameFailure);
			}
			else
			{
				std::fill_n(buttonIDLookup.ChainSlide[id].begin(), 4, Audio::SourceHandle::Invalid);
			}
		}

		for (u32 id = 0; id < buttonIDLookup.SliderTouch.size(); id++)
		{
			if (auto* entry = tryFindByID(Database::GmBtnSfxType::SliderTouch, id))
			{
				char nameBuffer[256];
				for (size_t i = 0; i < 32; i++)
					buttonIDLookup.SliderTouch[id][i] = findSfx(std::string_view(nameBuffer, sprintf_s(nameBuffer, "%s_%02zu", entry->SfxName.c_str(), i + 1)));
			}
			else
			{
				std::fill_n(buttonIDLookup.SliderTouch[id].begin(), 32, Audio::SourceHandle::Invalid);
			}
		}
	}
}
