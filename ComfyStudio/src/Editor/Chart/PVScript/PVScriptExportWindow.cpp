#include "PVScriptExportWindow.h"
#include "Core/ComfyStudioSettings.h"
#include "Core/ComfyStudioApplication.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include "Graphics/Utilities/SpritePacker.h"
#include "Graphics/Utilities/TextureCompression.h"
#include "Database/Game/GmPvListDB.h"
#include "Database/Game/PvDB.h"
#include "Resource/IDHash.h"
#include "Misc/StringUtil.h"
#include "Misc/UTF8.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "IO/Directory.h"
#include "IO/Archive/FArcPacker.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		void ConvertOrCopyToOggAndWaitForFinish(std::string_view sourcePath, std::string_view destinationPath)
		{
			if (!IO::File::Exists(sourcePath))
				return;

			if (IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(sourcePath), ".ogg"))
			{
				IO::File::Copy(sourcePath, destinationPath, true);
				return;
			}

			// TODO: Include libvorbis and encode using it...?
		}

		constexpr PVCommandLayout::TargetType ButtonTypeToPVCommandTargetType(const TimelineTarget& target)
		{
			using namespace PVCommandLayout;
			const bool isHold = target.Flags.IsHold;
			const bool isChain = target.Flags.IsChain;
			const bool isChance = target.Flags.IsChance;

			switch (target.Type)
			{
			case ButtonType::Triangle: return isHold ? TargetType::TriangleHold : isChance ? TargetType::TriangleChance : TargetType::Triangle;
			case ButtonType::Square: return isHold ? TargetType::SquareHold : isChance ? TargetType::SquareChance : TargetType::Square;
			case ButtonType::Cross: return isHold ? TargetType::CrossHold : isChance ? TargetType::CrossChance : TargetType::Cross;
			case ButtonType::Circle: return isHold ? TargetType::CircleHold : isChance ? TargetType::CircleChance : TargetType::Circle;
			case ButtonType::SlideL: return isChain ? TargetType::SlideChainL : isChance ? TargetType::SlideLChance : TargetType::SlideL;
			case ButtonType::SlideR: return isChain ? TargetType::SlideChainR : isChance ? TargetType::SlideRChance : TargetType::SlideR;
			}

			return TargetType::Circle;
		}

		PVScript ConvertChartToPVScript(const Chart& chart, bool addMovieCommands = false, vec4 backgroundTint = {})
		{
			PVScriptBuilder scriptBuilder {};

			scriptBuilder.Add(TimeSpan::Zero(), PVCommandLayout::ChangeField(1));
			scriptBuilder.Add(TimeSpan::Zero(), PVCommandLayout::MikuDisp(0, false));

			if (backgroundTint.a > 0.0f)
				scriptBuilder.Add(TimeSpan::Zero(), PVCommandLayout::SceneFade(TimeSpan::Zero(), backgroundTint.a, backgroundTint.a, vec3(backgroundTint)));

			const auto clampedStartOffsetCommandTime = std::max(-chart.StartOffset, TimeSpan::Zero());
			if (!chart.SongFileName.empty())
				scriptBuilder.Add(clampedStartOffsetCommandTime, PVCommandLayout::MusicPlay());

			if (addMovieCommands)
			{
				scriptBuilder.Add(clampedStartOffsetCommandTime, PVCommandLayout::MoviePlay(1));
				scriptBuilder.Add(clampedStartOffsetCommandTime, PVCommandLayout::MovieDisp(true));
			}

			const TimeSpan targetTimeDelay = std::max(chart.StartOffset, TimeSpan::Zero());
			i32 lastFlyDurationMS = {};

			for (const auto& target : chart.Targets)
			{
				const TimeSpan targetTime = chart.TimelineMap.GetTimeAt(target.Tick - BeatTick::FromBars(1)) + targetTimeDelay;
				const TimeSpan buttonTime = chart.TimelineMap.GetTimeAt(target.Tick) + targetTimeDelay;
				const i32 flyDurationMS = static_cast<i32>(glm::round((buttonTime - targetTime).TotalMilliseconds()));

				auto targetProperties = Rules::TryGetProperties(target);
				if (target.Flags.IsChain && !target.Flags.IsChainStart)
					targetProperties.Position.x += Rules::ChainFragmentStartEndOffsetDistance * (target.Type == ButtonType::SlideL ? -1.0f : +1.0f);

				auto targetCommand = PVCommandLayout::Target();
				targetCommand.Type = ButtonTypeToPVCommandTargetType(target);
				targetCommand.PositionX = static_cast<i32>(targetProperties.Position.x * 250.0f);
				targetCommand.PositionY = static_cast<i32>(targetProperties.Position.y * 250.0f);
				targetCommand.Angle = static_cast<i32>(targetProperties.Angle * 1000.0f);
				targetCommand.Distance = static_cast<i32>(targetProperties.Distance * 250.0f);
				targetCommand.Amplitude = static_cast<i32>(targetProperties.Amplitude);
				targetCommand.Frequency = static_cast<i32>(targetProperties.Frequency);

				if (flyDurationMS != lastFlyDurationMS)
				{
					scriptBuilder.Add(targetTime, PVCommandLayout::TargetFlyingTime(flyDurationMS));
					lastFlyDurationMS = flyDurationMS;
				}

				scriptBuilder.Add(targetTime, targetCommand);
			}

			scriptBuilder.Add(chart.DurationOrDefault(), PVCommandLayout::PVEnd());

			PVScript script = {};
			script.Commands = std::move(scriptBuilder.Create());
			return script;
		}

		std::unique_ptr<Graphics::SprSet> CreateChartSelPVSprSet(const Chart& chart, i32 pvID)
		{
			Graphics::Utilities::SpritePacker sprPacker = {};
			std::vector<Graphics::Utilities::SprMarkup> sprMarkups;
			sprMarkups.reserve(3);

			std::array<std::unique_ptr<u8[]>, 3> resizeBuffers = {};

			auto tryAdd = [&](Render::TexSprView texSprView, std::string_view name, ivec2 sizeHD720, std::unique_ptr<u8[]>& resizeBuffer)
			{
				if (!texSprView)
					return;

				const ivec2 existingSize = texSprView.Tex->GetSize();

				auto& markup = sprMarkups.emplace_back();
				markup.Name = name;
				markup.Flags = Graphics::Utilities::SprMarkupFlags_Compress;

				if ((existingSize.x * existingSize.y) > (sizeHD720.x * sizeHD720.y) && false)
				{
					markup.ScreenMode = Graphics::ScreenMode::HDTV1080;
					markup.Size = ivec2(vec2(sizeHD720) * 1.5f);
				}
				else
				{
					markup.ScreenMode = Graphics::ScreenMode::HDTV720;
					markup.Size = sizeHD720;
				}

				const auto resizeByteSize = Graphics::Utilities::TextureFormatByteSize(markup.Size, Graphics::TextureFormat::RGBA8);
				resizeBuffer = std::make_unique<u8[]>(resizeByteSize);

				// TODO: Transparent border
				if (markup.Size != existingSize)
				{
					Graphics::Utilities::ResizeTextureBuffer(
						existingSize,
						texSprView.Tex->MipMapsArray[0][0].Data.get(),
						texSprView.Tex->MipMapsArray[0][0].Format,
						texSprView.Tex->MipMapsArray[0][0].DataSize,
						markup.Size,
						resizeBuffer.get(),
						resizeByteSize);
				}
				else
				{
					std::memcpy(resizeBuffer.get(), texSprView.Tex->MipMapsArray[0][0].Data.get(), texSprView.Tex->MipMapsArray[0][0].DataSize);
				}

				Graphics::Utilities::FlipTextureBufferY(markup.Size, resizeBuffer.get(), Graphics::TextureFormat::RGBA8, resizeByteSize);
				markup.RGBAPixels = resizeBuffer.get();
			};

			// HACK: To get things up and runnig quickly
			Chart* hackyMutableChart = const_cast<Chart*>(&chart);
			char buffer[32];
			tryAdd(hackyMutableChart->Properties.Image.Cover.GetTexSprView(), std::string_view(buffer, sprintf_s(buffer, "SONG_JK%03d", pvID)), ivec2(500, 500), resizeBuffers[0]);
			tryAdd(hackyMutableChart->Properties.Image.Logo.GetTexSprView(), std::string_view(buffer, sprintf_s(buffer, "SONG_LOGO%03d", pvID)), ivec2(860, 300), resizeBuffers[1]);
			tryAdd(hackyMutableChart->Properties.Image.Background.GetTexSprView(), std::string_view(buffer, sprintf_s(buffer, "SONG_BG%03d", pvID)), ivec2(1280, 720), resizeBuffers[2]);

			auto sprSet = sprPacker.Create(sprMarkups);

			for (const auto& spr : sprSet->Sprites)
			{
				if (Util::StartsWith(spr.Name, "SONG_BG"))
				{
					sprSet->Sprites.emplace_back(spr).Name = "IMAGE";
					break;
				}
			}

			return sprSet;
		}

		void GenerateChartSelPVSprDBEntries(const Chart& chart, i32 pvID, Graphics::SprSet& sprSelPV, Database::SprDB& outSprDB)
		{
			char nameBuffer[64];

			auto& sprSetEntry = outSprDB.Entries.emplace_back();
			sprSetEntry.FileName.append(nameBuffer, sprintf_s(nameBuffer, "spr_sel_pv%03d.bin", pvID));
			sprSetEntry.Name.append(nameBuffer, sprintf_s(nameBuffer, "SPR_SEL_PV%03d", pvID));
			sprSetEntry.ID = HashIDString<SprSetID>(sprSetEntry.Name);

			sprSetEntry.SprEntries.reserve(sprSelPV.Sprites.size());
			for (auto& spr : sprSelPV.Sprites)
			{
				auto& sprEntry = sprSetEntry.SprEntries.emplace_back();
				sprEntry.Name.append(nameBuffer, sprintf_s(nameBuffer, "SPR_SEL_PV%03d_", pvID)).append(spr.Name);
				sprEntry.ID = HashIDString<SprID>(sprEntry.Name);
				sprEntry.Index = static_cast<i16>(std::distance(&sprSelPV.Sprites.front(), &spr));
			}

			sprSetEntry.SprTexEntries.reserve(sprSelPV.TexSet.Textures.size());
			for (auto& sprTex : sprSelPV.TexSet.Textures)
			{
				auto& sprTexEntry = sprSetEntry.SprTexEntries.emplace_back();
				sprTexEntry.Name.append(nameBuffer, sprintf_s(nameBuffer, "SPRTEX_SEL_PV%03d_", pvID)).append(sprTex->GetName());
				sprTexEntry.ID = HashIDString<SprID>(sprTexEntry.Name);
				sprTexEntry.Index = static_cast<i16>(std::distance(&sprSelPV.TexSet.Textures.front(), &sprTex));
			}
		}

		void ExportChartToMData(const PVScriptExportWindowInputData& inData, const PVScriptMDataExportParam& param)
		{
			constexpr Database::DateEntry startDate = { 2021, 1, 1 }, endDate = { 2029, 1, 1 };

			IO::Directory::Create(param.OutMDataDirectory);
			IO::Directory::Create(param.OutMDataRomDirectory);
			IO::Directory::Create(param.OutMDataRom2DDirectory);

			IO::File::WriteAllText(param.OutMDataInfo,
				"#mdata_info\n"
				"depend.length=0\n"
				"version=20161030\n");

			const auto songSourceFilePathAbsolute = IO::Path::ResolveRelativeTo(inData.Chart->SongFileName, inData.Chart->ChartFilePath);
			auto oggProcessFuture = std::async(std::launch::async, [&songSourceFilePathAbsolute, &param]() { ConvertOrCopyToOggAndWaitForFinish(songSourceFilePathAbsolute, param.OutOgg); });

			PVScript script = ConvertChartToPVScript(*inData.Chart, param.AddDummyMovieReference, param.PVScriptBackgroundTint);
			IO::File::Save(param.OutDsc, script);

			const auto[pvDifficulty, pvDifficultyEdition] = [](const Difficulty in) -> std::pair<Database::PVDifficultyType, Database::PVDifficultyEdition>
			{
				switch (in)
				{
				default:
				case Difficulty::Easy:
					return std::make_pair(Database::PVDifficultyType::Easy, Database::PVDifficultyEdition::Normal);
				case Difficulty::Normal:
					return std::make_pair(Database::PVDifficultyType::Normal, Database::PVDifficultyEdition::Normal);
				case Difficulty::Hard:
					return std::make_pair(Database::PVDifficultyType::Hard, Database::PVDifficultyEdition::Normal);
				case Difficulty::Extreme:
					return std::make_pair(Database::PVDifficultyType::Extreme, Database::PVDifficultyEdition::Normal);
				case Difficulty::ExExtreme:
					return std::make_pair(Database::PVDifficultyType::Extreme, Database::PVDifficultyEdition::Extra);
				}
			}(inData.Chart->Properties.Difficulty.Type);
			const char* pvDifficultyString = std::array { "easy", "normal", "hard", "extreme" }[static_cast<size_t>(pvDifficulty)];

			{
				auto findBtnSfxIDEntry = [&](u32 id, Database::GmBtnSfxType sfxType) -> const Database::GmBtnSfxEntry*
				{
					if (sfxType == Database::GmBtnSfxType::Button && id == 0)
						id = 1;

					for (const auto& entry : inData.SoundEffectManager->ViewSortedBtnSfxDB(sfxType))
					{
						if (entry->ID == id)
							return entry;
					}
					return nullptr;
				};

				std::string_view buttonSourceName, slideSourceName, chainSlideFirstName, chainSlideSubName, chainSlideSuccessName, chainSlideFailureName, sliderTouchName;

				if (const auto* entry = findBtnSfxIDEntry(inData.Chart->Properties.ButtonSound.ButtonID, Database::GmBtnSfxType::Button))
					buttonSourceName = entry->SfxName;
				if (const auto* entry = findBtnSfxIDEntry(inData.Chart->Properties.ButtonSound.SlideID, Database::GmBtnSfxType::Slide))
					slideSourceName = entry->SfxName;
				if (const auto* entry = findBtnSfxIDEntry(inData.Chart->Properties.ButtonSound.ChainSlideID, Database::GmBtnSfxType::ChainSlide))
				{
					chainSlideFirstName = entry->Chain.SfxNameFirst;
					chainSlideSubName = entry->Chain.SfxNameSub;
					chainSlideSuccessName = entry->Chain.SfxNameSuccess;
					chainSlideFailureName = entry->Chain.SfxNameFailure;
				}
				if (const auto* entry = findBtnSfxIDEntry(inData.Chart->Properties.ButtonSound.SliderTouchID, Database::GmBtnSfxType::SliderTouch))
					sliderTouchName = entry->SfxName;

				const bool chartHasSlides = std::any_of(inData.Chart->Targets.begin(), inData.Chart->Targets.end(), [](auto& t) { return IsSlideButtonType(t.Type); });

				char b[512];
				std::string pvDB;
				pvDB.reserve(0x800);
				pvDB.append("# --- COMFY STUDIO MDATA EXPORT: ---\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.bpm=%d\n", param.OutPVID, static_cast<i32>(inData.Chart->TempoMap.GetTempoChangeAt(0).Tempo.BeatsPerMinute)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_failure_name=", param.OutPVID)).append(chainSlideFailureName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_first_name=", param.OutPVID)).append(chainSlideFirstName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_sub_name=", param.OutPVID)).append(chainSlideSubName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_success_name=", param.OutPVID)).append(chainSlideSuccessName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.date=%04d%02d%02d\n", param.OutPVID, startDate.Year, startDate.Month, startDate.Day));

				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.attribute.extra=%d\n", param.OutPVID, pvDifficultyString, (pvDifficultyEdition == Database::PVDifficultyEdition::Extra)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.attribute.original=%d\n", param.OutPVID, pvDifficultyString, (pvDifficultyEdition != Database::PVDifficultyEdition::Extra)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.attribute.slide=%d\n", param.OutPVID, pvDifficultyString, chartHasSlides));

				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.edition=%d\n", param.OutPVID, pvDifficultyString, static_cast<i32>(pvDifficultyEdition)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.level=%s\n", param.OutPVID, pvDifficultyString, IndexOr(static_cast<u8>(inData.Chart->Properties.Difficulty.Level), Database::PVDifficultyLevelNames, "")));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.level_sort_index=%d\n", param.OutPVID, pvDifficultyString, 50));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.script_file_name=rom/", param.OutPVID, pvDifficultyString)).append(IO::Path::GetFileName(param.OutDsc)).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.script_format=0x%X\n", param.OutPVID, pvDifficultyString, script.Version));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.version=%d\n", param.OutPVID, pvDifficultyString, 1));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.length=%d\n", param.OutPVID, pvDifficultyString, 1));

				// pvDB.append(b, sprintf_s(b, "pv_%03d.field.%02d.stage=%s\n", param.OutPVID, 1, "STGTST007"));
				pvDB.append(b, sprintf_s(b, "pv_%03d.field.%02d.spr_set_back=%s%03d\n", param.OutPVID, 1, "SPR_SEL_PV", param.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.field.length=%d\n", param.OutPVID, 1));
				pvDB.append(b, sprintf_s(b, "pv_%03d.lyric.%03d=%s\n", param.OutPVID, 0, "DUMMY_LYRICS"));
				pvDB.append(b, sprintf_s(b, "pv_%03d.motion.01=CMN_POSE_DEFAULT_T\n", param.OutPVID));

				if (!param.AddDummyMovieReference) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.movie_file_name=rom/", param.OutPVID)).append(IO::Path::GetFileName(param.OutDsc, false)).append(".mp4").append("\n");
				if (!param.AddDummyMovieReference) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.movie_surface=FRONT\n", param.OutPVID));

				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.0.chara=MIK\n", param.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.0.pv_costume=1\n", param.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.0.type=VOCAL\n", param.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.num=1\n", param.OutPVID));

				if (inData.Chart->Properties.SongPreview.Duration <= TimeSpan::Zero()) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.sabi.play_time=%g\n", param.OutPVID, inData.Chart->Properties.SongPreview.Duration.TotalSeconds()));
				if (inData.Chart->Properties.SongPreview.Duration <= TimeSpan::Zero()) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.sabi.start_time=%g\n", param.OutPVID, inData.Chart->Properties.SongPreview.StartTime.TotalSeconds()));

				pvDB.append(b, sprintf_s(b, "pv_%03d.se_name=", param.OutPVID)).append(buttonSourceName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.slide_name=", param.OutPVID)).append(slideSourceName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.slidertouch_name=", param.OutPVID)).append(sliderTouchName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.song_file_name=rom/", param.OutPVID)).append(inData.Chart->SongFileName.empty() ? "dummy.ogg" : IO::Path::GetFileName(param.OutOgg)).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.song_name=", param.OutPVID)).append(inData.Chart->SongTitleOrDefault()).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.song_name_reading=%s\n", param.OutPVID, ""));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.arranger=%s\n", param.OutPVID, inData.Chart->Properties.Song.Arranger.c_str()));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.illustration=%s\n", param.OutPVID, ""));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.lyrics=%s\n", param.OutPVID, inData.Chart->Properties.Song.Lyricist.c_str()));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.music=%s\n", param.OutPVID, ""));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.pv_editor=%s\n", param.OutPVID, ""));
				pvDB.append("# --- COMFY STUDIO EXPORT END ---\n");

				if (param.MergeWithExistingMData && !param.InMergeBaseMDataPVDB.empty())
				{
					// TODO: Find right place to insert instead of always at the end
					std::string basePVDB = IO::File::ReadAllText(param.InMergeBaseMDataPVDB);

					std::string combinedPVDB = std::move(basePVDB);
					combinedPVDB += "\n";
					combinedPVDB += pvDB;

					IO::File::WriteAllText(param.OutMDataPVDB, combinedPVDB);
				}
				else
				{
					IO::File::WriteAllText(param.OutMDataPVDB, pvDB);
				}
			}

			{
				std::unique_ptr<Database::GmPvListDB> pvList;
				if (param.MergeWithExistingMData && !param.InMergeBasePVListFArc.empty())
					pvList = IO::File::Load<Database::GmPvListDB>(param.InMergeBasePVListFArc + "<gm_pv_list.bin>");
				if (pvList == nullptr)
					pvList = std::make_unique<Database::GmPvListDB>();

				auto& newPvListEntry = pvList->Entries.emplace_back();
				newPvListEntry.ID = param.OutPVID;
				newPvListEntry.Ignore = true;
				newPvListEntry.Name = inData.Chart->SongTitleOrDefault();
				newPvListEntry.AdvDemoStartDate = { 2009, 1, 1 };
				newPvListEntry.AdvDemoEndDate = { 2009, 1, 1 };
				auto& difficultyEditions = newPvListEntry.Difficulties[static_cast<size_t>(pvDifficulty)];
				difficultyEditions.Count = 1;
				difficultyEditions.Editions[0].StartDate = startDate;
				difficultyEditions.Editions[0].EndDate = endDate;
				difficultyEditions.Editions[0].Edition = pvDifficultyEdition;
				difficultyEditions.Editions[0].Version = 1;

				IO::FArcPacker farcPacker;
				farcPacker.AddFile("gm_pv_list.bin", *pvList);
				farcPacker.CreateFlushFArc(param.OutPVListFArc, false);
			}

			if (param.CreateSprSelPV)
			{
				auto selPVSprSet = CreateChartSelPVSprSet(*inData.Chart, param.OutPVID);

				std::unique_ptr<Database::SprDB> sprDB = nullptr;
				if (param.MergeWithExistingMData && !param.InMergeBaseMDataSprDB.empty())
					sprDB = IO::File::Load<Database::SprDB>(param.InMergeBaseMDataSprDB);
				if (sprDB == nullptr)
					sprDB = std::make_unique<Database::SprDB>();

				GenerateChartSelPVSprDBEntries(*inData.Chart, param.OutPVID, *selPVSprSet, *sprDB);

				IO::FArcPacker farcPacker;
				farcPacker.AddFile(sprDB->Entries.back().FileName, *selPVSprSet);
				farcPacker.CreateFlushFArc(param.OutSprSelPVFArc, true);

				IO::File::Save(param.OutMDataSprDB, *sprDB);
			}

			oggProcessFuture.get();
		}
	}
}

namespace Comfy::Studio::Editor
{
	namespace
	{
		struct MDataInfo
		{
			std::string Root;
			std::string_view ID;
		};

		class MDataResolver
		{
		public:
			MDataResolver(std::string_view rootDirectory, std::string_view mDataRootDirectory, std::array<char, 5> mdataIDToExclude)
			{
				IO::Directory::IterateDirectories(mDataRootDirectory, [&](const auto& mdataDirectory)
				{
					MDataInfo mData;
					mData.Root = IO::Path::Normalize(mdataDirectory);
					mData.ID = IO::Path::GetFileName(mData.Root, false);

					if (mData.ID.size() == 4 && mData.ID[0] == 'M' && memcmp(mData.ID.data(), mdataIDToExclude.data(), 4) != 0)
						available.push_back(std::move(mData));
				});

				available.push_back(MDataInfo { std::string(rootDirectory), "" });
				std::sort(available.begin(), available.end(), [](auto& a, auto& b) { return a.ID > b.ID; });
			}

		public:
			std::string ResolvePath(std::string_view relativePath) const
			{
				for (const auto& mData : available)
				{
					pathBuffer.clear();
					pathBuffer += mData.Root;
					pathBuffer += '/';
					pathBuffer += relativePath;

					if (IO::File::Exists(pathBuffer))
						return pathBuffer;
				}

				return "";
			}

		private:
			std::vector<MDataInfo> available;
			mutable std::string pathBuffer;
		};

		template <typename Func>
		bool PathTextInputWithBrowserButton(std::string& inOutPath, const char* hintText, Func onBrowserButton)
		{
			const auto& style = Gui::GetStyle();
			const auto buttonSize = Gui::GetFrameHeight();

			bool changesMade = false;

			Gui::PushID(&inOutPath);
			Gui::PushItemWidth(std::max(1.0f, (Gui::GetContentRegionAvailWidth() - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));
			changesMade |= Gui::InputTextWithHint("##PathTextInput", hintText, &inOutPath, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
			Gui::PopItemWidth();

			Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.y));
			Gui::SameLine(0, style.ItemInnerSpacing.x);
			if (Gui::Button("...", vec2(buttonSize)))
			{
				changesMade |= onBrowserButton(inOutPath);
			}
			Gui::PopStyleVar();
			Gui::PopID();

			return changesMade;
		}

		std::string OpenSelectFolderDialog(std::string_view title = "Select Folder")
		{
			IO::Shell::FileDialog fileDialog;
			fileDialog.Title = title;
			fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

			return fileDialog.OpenSelectFolder() ? IO::Path::Normalize(fileDialog.OutFilePath) : "";
		}
	}

	PVScriptExportWindow::PVScriptExportWindow()
	{
		param.OutFormat = PVScriptExportFormat::Arcade;
		param.OutMDataID = { "MXCY" };
		param.OutPVID = 911;
		param.PVScriptBackgroundTint = vec4(0.0f, 0.0f, 0.0f, 0.35f);
		param.MergeWithExistingMData = true;
		param.CreateSprSelPV = true;
	}

	void PVScriptExportWindow::Gui(const PVScriptExportWindowInputData& inData)
	{
		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		const auto& style = Gui::GetStyle();
		Gui::BeginChild("OutterChild", vec2(480.0f, 340.0f), true);
		{
			auto guiSameLineRightAlignedHintText = [](std::string_view description)
			{
				const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(description), Gui::StringViewEnd(description), false);
				Gui::SameLine(Gui::GetContentRegionAvailWidth() - textSize.x);
				Gui::PushStyleColor(ImGuiCol_Text, Gui::GetColorU32(ImGuiCol_TextDisabled));
				Gui::TextUnformatted(Gui::StringViewStart(description), Gui::StringViewEnd(description));
				Gui::PopStyleColor();
			};

			auto guiHeaderLabel = [&guiSameLineRightAlignedHintText](std::string_view label, std::string_view description = "")
			{
				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted(Gui::StringViewStart(label), Gui::StringViewEnd(label));
				if (!description.empty())
					guiSameLineRightAlignedHintText(description);
			};

			guiHeaderLabel("Export Target");
			{
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				if (Gui::BeginCombo("##ExportFormat", PVScriptExportFormatNames[static_cast<u8>(param.OutFormat)]))
				{
					for (size_t i = 0; i < EnumCount<PVScriptExportFormat>(); i++)
					{
						const auto format = static_cast<PVScriptExportFormat>(i);
						if (Gui::Selectable(PVScriptExportFormatNames[i], format == param.OutFormat, !IsPVScriptExportFormatSupported(format) ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
							param.OutFormat = format;
					}
					Gui::EndCombo();
				}
				Gui::PopItemWidth();
			}
			Gui::Separator();

			guiHeaderLabel("Game Root Directory", "(Containing executable and rom subdirectory)");
			{
				PathTextInputWithBrowserButton(param.RootDirectory, "\"...\" to select a folder", [&](std::string& inOutPath)
				{
					if (auto p = OpenSelectFolderDialog("Select Base Game Directory"); !p.empty())
					{
						inOutPath = p;
						return true;
					}
					else
					{
						return false;
					}
				});
			}
			Gui::Separator();

			guiHeaderLabel("Output MData ID", "(Expected to be highest priority)");
			{
				for (char& c : param.OutMDataID)
				{
					if (!(c >= '0' && c <= '9') && !(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z'))
						c = 'X';
				}
				param.OutMDataID[0] = 'M';
				param.OutMDataID[4] = '\0';
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				Gui::InputText("##MDataID", param.OutMDataID.data(), param.OutMDataID.size(), ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_AlwaysInsertMode);
				Gui::PopItemWidth();
			}
			Gui::Separator();

			guiHeaderLabel("Output PV ID", "(Expected to be unused)");
			{
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				if (i32 step = 1, stepFast = 100; Gui::InputScalar("##PVID", ImGuiDataType_S32, &param.OutPVID, &step, &stepFast, "%03d", ImGuiInputTextFlags_None))
					param.OutPVID = std::clamp(param.OutPVID, 1, 999);
				Gui::PopItemWidth();
			}
			Gui::Separator();

			guiHeaderLabel("Background Dim");
			Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
			if (auto v = param.PVScriptBackgroundTint.a * 100.0f; Gui::SliderFloat("##BackgroundDim", &v, 0.0f, 100.0f, "%.0f%%"))
				param.PVScriptBackgroundTint.a = v / 100.0f;
			Gui::PopItemWidth();
			Gui::Separator();

			Gui::Checkbox("Merge with Existing MData", &param.MergeWithExistingMData);
			Gui::Checkbox("Export Image Sprites", &param.CreateSprSelPV); guiSameLineRightAlignedHintText("(Cover, Logo, Background)");
			Gui::Checkbox("Dummy Movie Reference", &param.AddDummyMovieReference); guiSameLineRightAlignedHintText("(MP4 can manually be copied to output MData rom)");
			Gui::Separator();
		}
		Gui::EndChild();

		Gui::BeginChild("ConfirmatioBaseChild", vec2(0.0f, 32.0f), true, ImGuiWindowFlags_None);
		{
			if (Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !thisFrameAnyItemActive && !lastFrameAnyItemActive)
			{
				if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false))
				{
					StartExport(inData);
					RequestExit();
				}
				else if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
				{
					RequestExit();
				}
			}

			Gui::PushItemDisabledAndTextColorIf(param.RootDirectory.empty());
			if (Gui::Button("Export MData", vec2((Gui::GetContentRegionAvailWidth() - Gui::GetStyle().ItemSpacing.x) * 0.5f, Gui::GetContentRegionAvail().y)))
			{
				StartExport(inData);
				RequestExit();
			}
			Gui::PopItemDisabledAndTextColorIf(param.RootDirectory.empty());
			Gui::SameLine();
			if (Gui::Button("Cancel", Gui::GetContentRegionAvail()))
			{
				RequestExit();
			}
		}
		Gui::EndChild();
	}

	bool PVScriptExportWindow::GetAndClearCloseRequestThisFrame()
	{
		const bool result = closeWindowThisFrame;
		closeWindowThisFrame = false;
		return result;
	}

	void PVScriptExportWindow::OnWindowOpen()
	{
		InternalOnOpen();
	}

	void PVScriptExportWindow::OnCloseButtonClicked()
	{
		InternalOnClose();
	}

	void PVScriptExportWindow::ConvertAndSaveSimpleScriptSync(std::string_view outputScriptPath, const Chart& chart) const
	{
		if (outputScriptPath.empty())
			return;

		auto convertedPVScript = ConvertChartToPVScript(chart);
		IO::File::Save(outputScriptPath, convertedPVScript);
	}

	void PVScriptExportWindow::StartExport(const PVScriptExportWindowInputData& inData)
	{
		if (param.RootDirectory.empty() || !IO::Directory::Exists(param.RootDirectory))
			return;

		param.MDataRootDirectory = IO::Path::Combine(param.RootDirectory, "mdata");

		auto mDataResolver = MDataResolver(param.RootDirectory, param.MDataRootDirectory, param.OutMDataID);
		param.InMergeBasePVListFArc = mDataResolver.ResolvePath("rom/gm_pv_list_tbl.farc");
		param.InMergeBaseMDataPVDB = mDataResolver.ResolvePath("rom/mdata_pv_db.txt");
		param.InMergeBaseMDataSprDB = mDataResolver.ResolvePath("rom/2d/mdata_spr_db.bin");

		const std::string pvUnderscoreIDStr = [id = param.OutPVID]() { char b[16]; sprintf_s(b, "pv_%03d", id); return std::string(b); }();
		const std::string pvIDStr = [id = param.OutPVID]() { char b[16]; sprintf_s(b, "pv%03d", id); return std::string(b); }();

		param.OutMDataDirectory = IO::Path::Combine(param.MDataRootDirectory, param.OutMDataID.data());
		param.OutMDataRomDirectory = IO::Path::Combine(param.OutMDataDirectory, "rom");
		param.OutMDataRom2DDirectory = IO::Path::Combine(param.OutMDataRomDirectory, "2d");
		param.OutMDataInfo = IO::Path::Combine(param.OutMDataDirectory, "info.txt");
		param.OutOgg = IO::Path::Combine(param.OutMDataRomDirectory, pvUnderscoreIDStr + "_comfy.ogg");
		param.OutDsc = IO::Path::Combine(param.OutMDataRomDirectory, pvUnderscoreIDStr + "_comfy.dsc");
		param.OutPVListFArc = IO::Path::Combine(param.OutMDataRomDirectory, "gm_pv_list_tbl.farc");
		param.OutMDataPVDB = IO::Path::Combine(param.OutMDataRomDirectory, "mdata_pv_db.txt");
		param.OutMDataSprDB = IO::Path::Combine(param.OutMDataRom2DDirectory, "mdata_spr_db.bin");
		param.OutSprSelPVFArc = IO::Path::Combine(param.OutMDataRom2DDirectory, "spr_sel_" + pvIDStr + ".farc");

		ExportChartToMData(inData, param);
	}

	void PVScriptExportWindow::RequestExit()
	{
		closeWindowThisFrame = true;
		InternalOnClose();
	}

	// TODO: Load GlobalAppData state
	void PVScriptExportWindow::InternalOnOpen()
	{
		printf(__FUNCTION__"()\n");
	}

	// TODO: Save GlobalAppData state
	void PVScriptExportWindow::InternalOnClose()
	{
		printf(__FUNCTION__"()\n");
	}
}
