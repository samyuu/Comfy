#include "PVScriptExportWindow.h"
#include "Core/ComfyStudioSettings.h"
#include "Core/ComfyStudioApplication.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include "Graphics/Utilities/SpritePacker.h"
#include "Graphics/Utilities/TextureCompression.h"
#include "Audio/Decoder/DecoderFactory.h"
#include "Audio/Encoder/EncoderUtil.h"
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
		bool HasOggExtension(std::string_view filePath)
		{
			return IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(filePath), ".ogg");
		}

		void ConvertOrCopyToOgg(std::string_view sourcePath, std::string_view destinationPath, std::shared_ptr<Audio::ISampleProvider> songSampleProvider, f32 vbrQuality, std::atomic<f32>& outProgress)
		{
			if (sourcePath.empty() || !IO::File::Exists(sourcePath))
				return;

			if (HasOggExtension(sourcePath))
			{
				IO::File::Copy(sourcePath, destinationPath, true);
				return;
			}

			using namespace Comfy::Audio;

			std::shared_ptr<ISampleProvider> inputFile = (songSampleProvider == nullptr) ? DecoderFactory::GetInstance().DecodeFile(sourcePath) : songSampleProvider;
			if (inputFile == nullptr)
				return;

			auto outputFileStream = IO::File::CreateWrite(destinationPath);
			if (!outputFileStream.IsOpen() || !outputFileStream.CanWrite())
				return;

			i64 framesReadSoFar = 0;

			EncoderInput input = {};
			input.ChannelCount = inputFile->GetChannelCount();
			input.SampleRate = inputFile->GetSampleRate();
			input.TotalFrameCount = inputFile->GetFrameCount();
			input.ReadRawSamples = [&](i64 framesToRead, i16* bufferToFill) { inputFile->ReadSamples(bufferToFill, framesReadSoFar, framesToRead); framesReadSoFar += framesToRead; };

			EncoderOutput output = {};
			output.WriteFileBytes = [&](size_t byteSize, const u8* bytesToWrite) { outputFileStream.WriteBuffer(bytesToWrite, byteSize); };

			EncoderOptions options = {};
			options.VBRQuality = vbrQuality;

			EncoderCallbacks callbacks = {};
			callbacks.OnSamplesEncoded = [&outProgress](const EncoderCallbackProgressStatus& progressStatus)
			{
				outProgress = static_cast<f32>(static_cast<f64>(progressStatus.FramesEncodedSoFar) / static_cast<f64>(progressStatus.TotalFramesToEncode));
				return EncoderCallbackResponse::Continue;
			};

			const auto encoderResult = EncodeOggVorbis(input, output, options, callbacks);
			return;
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

		constexpr std::pair<Database::PVDifficultyType, Database::PVDifficultyEdition> DifficultyToPVDifficultyTypeAndEdition(const Difficulty difficulty)
		{
			switch (difficulty)
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
		}

		PVScript ConvertChartToPVScript(const Chart& chart, bool addMovieCommands = false, vec4 backgroundTint = {})
		{
			PVScriptBuilder scriptBuilder {};

			scriptBuilder.Add(TimeSpan::Zero(), PVCommandLayout::ChangeField(1));
			scriptBuilder.Add(TimeSpan::Zero(), PVCommandLayout::MikuDisp(0, false));

			if (backgroundTint.a > 0.0f)
				scriptBuilder.Add(TimeSpan::Zero(), PVCommandLayout::SceneFade(TimeSpan::Zero(), backgroundTint.a, backgroundTint.a, vec3(backgroundTint)));

			const auto clampedSongOffsetCommandTime = std::max(-chart.SongOffset, TimeSpan::Zero());
			if (!chart.SongFileName.empty())
				scriptBuilder.Add(clampedSongOffsetCommandTime, PVCommandLayout::MusicPlay());

			if (addMovieCommands)
			{
				// TODO: Handle movie offset
				scriptBuilder.Add(clampedSongOffsetCommandTime, PVCommandLayout::MoviePlay(1));
				scriptBuilder.Add(clampedSongOffsetCommandTime, PVCommandLayout::MovieDisp(true));
			}

			const TimeSpan targetTimeDelay = std::max(chart.SongOffset, TimeSpan::Zero());
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

		void AsyncTasksExportChartToMData(const PVExportWindowInputData& inData, const PVMDataExportParam& inParam, PVExportTasks& outTasks, PVExportAtomicProgress& outProgress)
		{
			static constexpr Database::DateEntry startDate = { 2021, 1, 1 }, endDate = { 2029, 1, 1 };

			IO::Directory::Create(inParam.OutMDataDirectory);
			IO::Directory::Create(inParam.OutMDataRomDirectory);
			IO::Directory::Create(inParam.OutMDataRom2DDirectory);

			outTasks.push_back(std::async(std::launch::async, [&inData, &inParam, &outProgress]()
			{
				IO::File::WriteAllText(inParam.OutMDataInfo,
					"#mdata_info\n"
					"depend.length=0\n"
					"version=20161030\n");
				outProgress.MDataInfo = 1.0f;
			}));

			outTasks.push_back(std::async(std::launch::async, [&inData, &inParam, &outProgress]()
			{
				const auto absoluteSongFilePath = IO::Path::ResolveRelativeTo(inData.Chart->SongFileName, inData.Chart->ChartFilePath);
				ConvertOrCopyToOgg(absoluteSongFilePath, inParam.OutOgg, inParam.SongSampleProvider, inParam.VorbisVBRQuality, outProgress.Audio);
				outProgress.Audio = 1.0f;
			}));

			outTasks.push_back(std::async(std::launch::async, [&inData, &inParam, &outProgress]()
			{
				PVScript script = ConvertChartToPVScript(*inData.Chart, inParam.AddDummyMovieReference, inParam.PVScriptBackgroundTint);
				IO::File::Save(inParam.OutDsc, script);
				outProgress.Script = 1.0f;
			}));

			outTasks.push_back(std::async(std::launch::async, [&inData, &inParam, &outProgress]()
			{
				const auto[pvDifficulty, pvDifficultyEdition] = DifficultyToPVDifficultyTypeAndEdition(inData.Chart->Properties.Difficulty.Type);
				const char* pvDifficultyString = std::array { "easy", "normal", "hard", "extreme" }[static_cast<size_t>(pvDifficulty)];

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
				pvDB.append("\n\n");
				pvDB.append("# --- COMFY STUDIO MDATA EXPORT: ---\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.bpm=%d\n", inParam.OutPVID, static_cast<i32>(inData.Chart->TempoMap.GetTempoChangeAt(0).Tempo.BeatsPerMinute)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_failure_name=", inParam.OutPVID)).append(chainSlideFailureName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_first_name=", inParam.OutPVID)).append(chainSlideFirstName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_sub_name=", inParam.OutPVID)).append(chainSlideSubName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.chainslide_success_name=", inParam.OutPVID)).append(chainSlideSuccessName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.date=%04d%02d%02d\n", inParam.OutPVID, startDate.Year, startDate.Month, startDate.Day));

				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.attribute.extra=%d\n", inParam.OutPVID, pvDifficultyString, (pvDifficultyEdition == Database::PVDifficultyEdition::Extra)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.attribute.original=%d\n", inParam.OutPVID, pvDifficultyString, (pvDifficultyEdition != Database::PVDifficultyEdition::Extra)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.attribute.slide=%d\n", inParam.OutPVID, pvDifficultyString, chartHasSlides));

				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.edition=%d\n", inParam.OutPVID, pvDifficultyString, static_cast<i32>(pvDifficultyEdition)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.level=%s\n", inParam.OutPVID, pvDifficultyString, IndexOr(static_cast<u8>(inData.Chart->Properties.Difficulty.Level), Database::PVDifficultyLevelNames, "")));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.level_sort_index=%d\n", inParam.OutPVID, pvDifficultyString, 50));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.script_file_name=rom/", inParam.OutPVID, pvDifficultyString)).append(IO::Path::GetFileName(inParam.OutDsc)).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.script_format=0x%X\n", inParam.OutPVID, pvDifficultyString, static_cast<u32>(PVScriptVersion::Current)));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.0.version=%d\n", inParam.OutPVID, pvDifficultyString, 1));
				pvDB.append(b, sprintf_s(b, "pv_%03d.difficulty.%s.length=%d\n", inParam.OutPVID, pvDifficultyString, 1));

				// pvDB.append(b, sprintf_s(b, "pv_%03d.field.%02d.stage=%s\n", param.OutPVID, 1, "STGTST007"));
				pvDB.append(b, sprintf_s(b, "pv_%03d.field.%02d.spr_set_back=%s%03d\n", inParam.OutPVID, 1, "SPR_SEL_PV", inParam.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.field.length=%d\n", inParam.OutPVID, 1));
				pvDB.append(b, sprintf_s(b, "pv_%03d.lyric.%03d=%s\n", inParam.OutPVID, 0, "DUMMY_LYRICS"));
				pvDB.append(b, sprintf_s(b, "pv_%03d.motion.01=CMN_POSE_DEFAULT_T\n", inParam.OutPVID));

				if (!inParam.AddDummyMovieReference) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.movie_file_name=rom/", inParam.OutPVID)).append(IO::Path::GetFileName(inParam.OutDsc, false)).append(".mp4").append("\n");
				if (!inParam.AddDummyMovieReference) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.movie_surface=FRONT\n", inParam.OutPVID));

				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.0.chara=MIK\n", inParam.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.0.pv_costume=1\n", inParam.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.0.type=VOCAL\n", inParam.OutPVID));
				pvDB.append(b, sprintf_s(b, "pv_%03d.performer.num=1\n", inParam.OutPVID));

				if (inData.Chart->Properties.SongPreview.Duration <= TimeSpan::Zero()) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.sabi.play_time=%g\n", inParam.OutPVID, inData.Chart->Properties.SongPreview.Duration.TotalSeconds()));
				if (inData.Chart->Properties.SongPreview.Duration <= TimeSpan::Zero()) pvDB.append("#");
				pvDB.append(b, sprintf_s(b, "pv_%03d.sabi.start_time=%g\n", inParam.OutPVID, inData.Chart->Properties.SongPreview.StartTime.TotalSeconds()));

				pvDB.append(b, sprintf_s(b, "pv_%03d.se_name=", inParam.OutPVID)).append(buttonSourceName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.slide_name=", inParam.OutPVID)).append(slideSourceName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.slidertouch_name=", inParam.OutPVID)).append(sliderTouchName).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.song_file_name=rom/", inParam.OutPVID)).append(inData.Chart->SongFileName.empty() ? "dummy.ogg" : IO::Path::GetFileName(inParam.OutOgg)).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.song_name=", inParam.OutPVID)).append(inData.Chart->SongTitleOrDefault()).append("\n");
				pvDB.append(b, sprintf_s(b, "pv_%03d.song_name_reading=%s\n", inParam.OutPVID, ""));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.arranger=%s\n", inParam.OutPVID, inData.Chart->Properties.Song.Arranger.c_str()));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.illustration=%s\n", inParam.OutPVID, ""));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.lyrics=%s\n", inParam.OutPVID, inData.Chart->Properties.Song.Lyricist.c_str()));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.music=%s\n", inParam.OutPVID, ""));
				pvDB.append(b, sprintf_s(b, "pv_%03d.songinfo.pv_editor=%s\n", inParam.OutPVID, ""));
				pvDB.append("# --- COMFY STUDIO EXPORT END ---\n");

				outProgress.PVDB = 0.1f;

				if (inParam.MergeWithExistingMData && !inParam.InMergeBaseMDataPVDB.empty())
				{
					auto forEachPVDBLine = [](std::string_view pvDB, auto perLineReturnFalseToStopFunc)
					{
						for (size_t absoulteIndex = 0; absoulteIndex < pvDB.size(); absoulteIndex++)
						{
							const std::string_view remainingPVDB = pvDB.substr(absoulteIndex);
							for (size_t relativeIndex = 0; relativeIndex < remainingPVDB.size(); relativeIndex++)
							{
								if (remainingPVDB[relativeIndex] == '\n')
								{
									const std::string_view line = remainingPVDB.substr(0, relativeIndex);
									if (relativeIndex + 1 < remainingPVDB.size() && remainingPVDB[relativeIndex + 1] == '\r')
										relativeIndex++;

									if (!line.empty() && line[0] != '#' && !perLineReturnFalseToStopFunc(line))
										return;

									absoulteIndex += relativeIndex;
									break;
								}
							}
						}
					};

					std::string basePVDB = IO::File::ReadAllText(inParam.InMergeBaseMDataPVDB);
					outProgress.PVDB = 0.5f;

					char pvUnderscoreIDBuffer[16];
					const auto pvUnderscoreIDStr = std::string_view(pvUnderscoreIDBuffer, sprintf_s(pvUnderscoreIDBuffer, "pv_%03d", inParam.OutPVID));

					std::string_view lineToInsertAfter = {};
					forEachPVDBLine(basePVDB, [&](std::string_view line)
					{
						if (line.size() > 6 && Util::StartsWith(line, "pv_"))
						{
							if (line.substr(0, 6) < pvUnderscoreIDStr)
								lineToInsertAfter = line;
							else
								return false;
						}

						return true;
					});

					std::string combinedPVDB = std::move(basePVDB);
					if (lineToInsertAfter.data() != nullptr)
					{
						const size_t insertionIndex = static_cast<size_t>(lineToInsertAfter.data() - &combinedPVDB.front()) + lineToInsertAfter.size();
						combinedPVDB.insert(insertionIndex, pvDB);
					}
					else
					{
						combinedPVDB.append(pvDB);
					}
					outProgress.PVDB = 0.8f;

					IO::File::WriteAllText(inParam.OutMDataPVDB, combinedPVDB);
				}
				else
				{
					IO::File::WriteAllText(inParam.OutMDataPVDB, pvDB);
				}
				outProgress.PVDB = 1.0f;
			}));

			outTasks.push_back(std::async(std::launch::async, [&inData, &inParam, &outProgress]()
			{
				const auto[pvDifficulty, pvDifficultyEdition] = DifficultyToPVDifficultyTypeAndEdition(inData.Chart->Properties.Difficulty.Type);

				std::unique_ptr<Database::GmPvListDB> pvList;
				if (inParam.MergeWithExistingMData && !inParam.InMergeBasePVListFArc.empty())
					pvList = IO::File::Load<Database::GmPvListDB>(inParam.InMergeBasePVListFArc + "<gm_pv_list.bin>");
				if (pvList == nullptr)
					pvList = std::make_unique<Database::GmPvListDB>();
				outProgress.PVList = 0.3f;

				auto& newPvListEntry = pvList->Entries.emplace_back();
				newPvListEntry.ID = inParam.OutPVID;
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
				outProgress.PVList = 0.6f;
				farcPacker.CreateFlushFArc(inParam.OutPVListFArc, false);
				outProgress.PVList = 1.0f;
			}));

			outTasks.push_back(std::async(std::launch::async, [&inData, &inParam, &outProgress]()
			{
				if (inParam.CreateSprSelPV)
				{
					// TODO: Update progress inside...?
					auto selPVSprSet = CreateChartSelPVSprSet(*inData.Chart, inParam.OutPVID);
					outProgress.Sprites = 0.5f;

					std::unique_ptr<Database::SprDB> sprDB = nullptr;
					if (inParam.MergeWithExistingMData && !inParam.InMergeBaseMDataSprDB.empty())
						sprDB = IO::File::Load<Database::SprDB>(inParam.InMergeBaseMDataSprDB);
					if (sprDB == nullptr)
						sprDB = std::make_unique<Database::SprDB>();

					GenerateChartSelPVSprDBEntries(*inData.Chart, inParam.OutPVID, *selPVSprSet, *sprDB);
					outProgress.Sprites = 0.7f;

					IO::FArcPacker farcPacker;
					farcPacker.AddFile(sprDB->Entries.back().FileName, *selPVSprSet);
					farcPacker.CreateFlushFArc(inParam.OutSprSelPVFArc, true);
					outProgress.Sprites = 0.9f;

					IO::File::Save(inParam.OutMDataSprDB, *sprDB);
					outProgress.Sprites = 1.0f;
				}
				else
				{
					outProgress.Sprites = 1.0f;
				}
			}));
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
			Gui::PushItemWidth(std::max(1.0f, (Gui::GetContentRegionAvail().x - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));
			changesMade |= Gui::PathInputTextWithHint("##PathTextInput", hintText, &inOutPath, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_ReadOnly);
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
		param.OutFormat = PVExportFormat::Arcade;
		param.OutMDataID = { "MYEP" };
		param.OutPVID = 911;
		param.PVScriptBackgroundTint = vec4(0.0f, 0.0f, 0.0f, 0.35f);
		param.MergeWithExistingMData = true;
		param.CreateSprSelPV = true;
		param.AddDummyMovieReference = false;
		// TODO: What should be the default (?)
		param.VorbisVBRQuality = 0.8f;
	}

	PVScriptExportWindow::~PVScriptExportWindow()
	{
		tasks.clear();
	}

	void PVScriptExportWindow::Gui(const PVExportWindowInputData& inData)
	{
		assert(isCurrentlyAsyncExporting == !tasks.empty());
		if (isCurrentlyAsyncExporting)
			Gui::SetActiveID(Gui::GetID(this), Gui::GetCurrentWindow());

		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		const bool itemsDisabledDueToExport = isCurrentlyAsyncExporting;
		Gui::PushItemDisabledAndTextColorIf(itemsDisabledDueToExport);
		{
			const auto& style = Gui::GetStyle();
			Gui::BeginChild("OutterChild", vec2(480.0f, 386.0f), true);
			{
				auto guiSameLineRightAlignedHintText = [](std::string_view description)
				{
					const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(description), Gui::StringViewEnd(description), false);
					Gui::SameLine(Gui::GetContentRegionAvail().x - textSize.x);
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
					Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
					if (Gui::BeginCombo("##ExportFormat", PVExportFormatNames[static_cast<u8>(param.OutFormat)]))
					{
						for (size_t i = 0; i < EnumCount<PVExportFormat>(); i++)
						{
							const auto format = static_cast<PVExportFormat>(i);
							if (Gui::Selectable(PVExportFormatNames[i], format == param.OutFormat, !IsPVExportFormatSupported(format) ? ImGuiSelectableFlags_Disabled : ImGuiSelectableFlags_None))
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
						if (auto p = OpenSelectFolderDialog("Select Root Directory"); !p.empty())
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
					static auto isValidIDChar = [](char c) { return !(!(c >= '0' && c <= '9') && !(c >= 'A' && c <= 'Z') && !(c >= 'a' && c <= 'z')); };
					static auto validIDCharTextCallbackFilter = [](ImGuiInputTextCallbackData* data) -> int { return !isValidIDChar(static_cast<char>(data->EventChar)); };

					for (char& c : param.OutMDataID)
						c = isValidIDChar(c) ? c : '0';
					param.OutMDataID[0] = 'M';
					param.OutMDataID[4] = '\0';
					Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
					Gui::InputText("##MDataID", param.OutMDataID.data(), param.OutMDataID.size(),
						ImGuiInputTextFlags_CharsUppercase |
						ImGuiInputTextFlags_CharsNoBlank |
						ImGuiInputTextFlags_AutoSelectAll |
						ImGuiInputTextFlags_CallbackCharFilter |
						ImGuiInputTextFlags_AlwaysOverwrite,
						validIDCharTextCallbackFilter);
					Gui::PopItemWidth();
				}
				Gui::Separator();

				guiHeaderLabel("Output PV ID", "(Expected to be unused)");
				{
					Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
					if (i32 step = 1, stepFast = 100; Gui::InputScalar("##PVID", ImGuiDataType_S32, &param.OutPVID, &step, &stepFast, "%03d", ImGuiInputTextFlags_None))
						param.OutPVID = std::clamp(param.OutPVID, 1, 999);
					Gui::PopItemWidth();
				}
				Gui::Separator();

				guiHeaderLabel("Background Dim");
				Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
				if (auto v = param.PVScriptBackgroundTint.a * 100.0f; Gui::SliderFloat("##BackgroundDim", &v, 0.0f, 100.0f, "%.0f%%"))
					param.PVScriptBackgroundTint.a = v / 100.0f;
				Gui::PopItemWidth();
				Gui::Separator();

				Gui::Checkbox("Merge with Existing MData", &param.MergeWithExistingMData);
				Gui::Checkbox("Export Image Sprites", &param.CreateSprSelPV); guiSameLineRightAlignedHintText("(Cover, Logo, Background)");
				// TODO: Handle this better
				Gui::Checkbox("Dummy Movie Reference", &param.AddDummyMovieReference); guiSameLineRightAlignedHintText("(MP4 can manually be copied to output MData rom)");
				Gui::Separator();

				const bool isOggFile = (inData.Chart->SongFileName.empty() || HasOggExtension(inData.Chart->SongFileName));
				Gui::PushItemDisabledAndTextColorIf(isOggFile);
				guiHeaderLabel("Ogg Vorbis VBR Quality");
				Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
				if (auto v = param.VorbisVBRQuality * 10.0f; Gui::SliderFloat("##VorbisVBRQuality", &v, Audio::VorbisVBRQualityMin * 10.0f, Audio::VorbisVBRQualityMax * 10.0f, "q%.1f"))
					param.VorbisVBRQuality = v / 10.0f;
				Gui::PopItemWidth();
				Gui::PopItemDisabledAndTextColorIf(isOggFile);
				Gui::Separator();
			}
			Gui::EndChild();

			Gui::BeginChild("ConfirmatioBaseChild", vec2(0.0f, 32.0f), true, ImGuiWindowFlags_None);
			{
				if (Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !thisFrameAnyItemActive && !lastFrameAnyItemActive)
				{
					if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false))
					{
						StartAsyncExport(inData);
					}
					else if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
					{
						RequestExit();
					}
				}

				Gui::PushItemDisabledAndTextColorIf(param.RootDirectory.empty());
				if (Gui::Button("Export MData", vec2((Gui::GetContentRegionAvail().x - Gui::GetStyle().ItemSpacing.x) * 0.5f, Gui::GetContentRegionAvail().y)))
				{
					StartAsyncExport(inData);
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
		Gui::PopItemDisabledAndTextColorIf(itemsDisabledDueToExport);

		if (isCurrentlyAsyncExporting)
		{
			constexpr const char* progressWindowID = "Exporting PV Script MData";
			if (!Gui::IsPopupOpen(progressWindowID))
			{
				Gui::OpenPopup(progressWindowID);
				loadingAnimation.Reset();
			}

			const auto* viewport = Gui::GetMainViewport();
			Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Always, vec2(0.5f));

			if (Gui::WideBeginPopupModal(progressWindowID, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				// TODO: Make this look nicer, the "Loading" text animation also doesn't really fit too well..?
				Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(6.0f));
				Gui::BeginChild("ProgressChild", vec2(540.0f, 320.0f), true);
				{
					loadingAnimation.Update();
					Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Gui::GetStyle().ItemSpacing * 2.0f);
					Gui::TextUnformatted("  This may take a moment. Please Wait.");
					Gui::TextUnformatted(loadingAnimation.GetText());
					Gui::Separator();
					Gui::PopStyleVar();

					// TODO: Maybe add some interpolation to smooth out the progress bars..?
					Gui::TextUnformatted("  Audio"); Gui::ProgressBar(progress.Audio);
					Gui::TextUnformatted("  Sprite"); Gui::ProgressBar(progress.Sprites);
					Gui::TextUnformatted("  Script"); Gui::ProgressBar(progress.Script);
					Gui::TextUnformatted("  MData Info"); Gui::ProgressBar(progress.MDataInfo);
					Gui::TextUnformatted("  PV DB"); Gui::ProgressBar(progress.PVDB);
					Gui::TextUnformatted("  PV List"); Gui::ProgressBar(progress.PVList);
				}
				Gui::EndChild();
				Gui::PopStyleVar();

				if (std::all_of(tasks.begin(), tasks.end(), [](auto& future) { return future.valid() && future._Is_ready(); }))
				{
					Gui::CloseCurrentPopup();

					tasks.clear();
					progress.Reset();

					isCurrentlyAsyncExporting = false;
					RequestExit();
				}

				Gui::EndPopup();
			}
		}
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

	void PVScriptExportWindow::StartAsyncExport(PVExportWindowInputData inData)
	{
		assert(tasks.empty() && !isCurrentlyAsyncExporting);
		if (param.RootDirectory.empty() || !IO::Directory::Exists(param.RootDirectory))
			return;

		tasks.clear();
		progress.Reset();
		isCurrentlyAsyncExporting = true;
		asyncAccessedWindowInputData = inData;

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
		param.SongSampleProvider = asyncAccessedWindowInputData.SongSampleProvider;

		AsyncTasksExportChartToMData(asyncAccessedWindowInputData, param, tasks, progress);
	}

	void PVScriptExportWindow::RequestExit()
	{
		closeWindowThisFrame = true;
		InternalOnClose();
	}

	void PVScriptExportWindow::InternalOnOpen()
	{
		const auto& in = GlobalAppData.LastPVScriptExportOptions;
		auto& out = param;

		if (in.ExportFormatIndex.has_value())
			out.OutFormat = static_cast<PVExportFormat>(in.ExportFormatIndex.value());

		if (in.PVID.has_value())
			out.OutPVID = in.PVID.value();

		if (in.RootDirectory.has_value())
			out.RootDirectory = in.RootDirectory.value();

		if (in.MDataID.has_value() && in.MDataID->size() >= (out.OutMDataID.size() - 1))
		{
			for (size_t i = 0; i < 4; i++)
				out.OutMDataID[i] = in.MDataID->at(i);
			out.OutMDataID[4] = '\0';
		}

		if (in.BackgroundDim.has_value())
			out.PVScriptBackgroundTint = vec4(0.0f, 0.0f, 0.0f, in.BackgroundDim.value());

		if (in.MergeWithExistingMData.has_value())
			out.MergeWithExistingMData = in.MergeWithExistingMData.value();

		if (in.CreateSprSelPV.has_value())
			out.CreateSprSelPV = in.CreateSprSelPV.value();

		if (in.AddDummyMovieReference.has_value())
			out.AddDummyMovieReference = in.AddDummyMovieReference.value();

		if (in.VorbisVBRQuality.has_value())
			out.VorbisVBRQuality = in.VorbisVBRQuality.value();
	}

	// BUG: This isn't being called when closing the whole application while the export window is still open
	void PVScriptExportWindow::InternalOnClose()
	{
		tasks.clear();

		const auto& in = param;
		auto& out = GlobalAppData.LastPVScriptExportOptions;

		out.ExportFormatIndex = static_cast<i32>(in.OutFormat);
		out.PVID = in.OutPVID;
		out.RootDirectory = in.RootDirectory;
		out.MDataID = std::string_view(in.OutMDataID.data(), in.OutMDataID.size() - 1);
		out.BackgroundDim = in.PVScriptBackgroundTint.a;
		out.MergeWithExistingMData = in.MergeWithExistingMData;
		out.CreateSprSelPV = in.CreateSprSelPV;
		out.AddDummyMovieReference = in.AddDummyMovieReference;
		out.VorbisVBRQuality = in.VorbisVBRQuality;
	}
}
