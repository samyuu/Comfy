#include "ChartPropertiesWindow.h"
#include "ChartEditor.h"
#include "ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "IO/Path.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr TimeSpan ChainSlidePreviewChainDuration = TimeSpan::FromSeconds(/*1.0*/0.5);
		constexpr TimeSpan ChainSlidePreviewFadeOutDuration = TimeSpan::FromMilliseconds(40.0);

		constexpr TimeSpan SongPreviewFadeInDuration = TimeSpan::FromSeconds(0.25);
		constexpr TimeSpan SongPreviewFadeOutDuration = TimeSpan::FromSeconds(0.5);
		constexpr TimeSpan SongPreviewLoopDelay = TimeSpan::FromSeconds(0.35);

		bool GuiPropertyButtonSoundCombo(std::string_view label, u32& inOutID, Database::GmBtnSfxType btnSfxDBType, SoundEffectManager& soundEffectManager, Stopwatch& lastChainSlidePreviewStopwatch)
		{
			auto entryToCStr = [&](const Database::GmBtnSfxEntry* entry)
			{
				if (entry == nullptr)
					return "None";

#if COMFY_DEBUG // DEBUG: In case of the full font character set not having been loaded
				return (btnSfxDBType == Database::GmBtnSfxType::ChainSlide) ? entry->Chain.SfxNameFirst.c_str() : entry->SfxName.c_str();
#else
				return entry->DisplayName.c_str();
#endif
			};

			auto previewButtonSound = [&](const Database::GmBtnSfxEntry& entry)
			{
				auto& engine = Audio::AudioEngine::GetInstance();
				engine.EnsureStreamRunning();

				char nameBuffer[512];
				auto formatVoiceName = [&nameBuffer](std::string_view sfxName) -> std::string_view
				{
					return std::string_view(nameBuffer, sprintf_s(nameBuffer, "ButtonSoundPreview %.*s", static_cast<int>(sfxName.size()), sfxName.data()));
				};

				switch (btnSfxDBType)
				{
				case Database::GmBtnSfxType::Button:
					engine.PlayOneShotSound(soundEffectManager.GetButtonSound(entry.ID), formatVoiceName(entry.SfxName));
					break;

				case Database::GmBtnSfxType::Slide:
					engine.PlayOneShotSound(soundEffectManager.GetSlideSound(entry.ID), formatVoiceName(entry.SfxName));
					break;

				case Database::GmBtnSfxType::ChainSlide:
				{
					if (!lastChainSlidePreviewStopwatch.IsRunning() || lastChainSlidePreviewStopwatch.GetElapsed() >= ChainSlidePreviewChainDuration)
					{
						lastChainSlidePreviewStopwatch.Restart();
						const auto[firstSource, subSource, successSource, failureSource] = soundEffectManager.GetChainSlideSound(entry.ID);

						Audio::Voice startVoice = engine.AddVoice(firstSource, formatVoiceName(entry.Chain.SfxNameFirst), true);
						startVoice.SetRemoveOnEnd(true);
						startVoice.SetVolumeMap(ChainSlidePreviewChainDuration, ChainSlidePreviewChainDuration + ChainSlidePreviewFadeOutDuration, 1.0f, 0.0f);

						Audio::Voice endVoice = engine.AddVoice(successSource, formatVoiceName(entry.Chain.SfxNameSuccess), true);
						endVoice.SetRemoveOnEnd(true);
						endVoice.SetPosition(-ChainSlidePreviewChainDuration);
					}
					break;
				}

				case Database::GmBtnSfxType::SliderTouch:
				{
					constexpr size_t voicesToPlay = 16;
					constexpr auto voiceInterval = TimeSpan::FromSeconds(1.0 / 32.0);

					auto startOffset = TimeSpan::Zero();
					auto sources = soundEffectManager.GetSliderTouchSound(entry.ID);

					for (size_t i = 0; i < voicesToPlay; i++)
					{
						Audio::Voice voice = engine.AddVoice(sources[(sources.size() / voicesToPlay) * i], formatVoiceName(entry.SfxName), true);
						voice.SetRemoveOnEnd(true);
						voice.SetPosition(startOffset);
						startOffset -= voiceInterval;
					}
					break;
				}
				}
			};

			return GuiProperty::PropertyLabelValueFunc(label, [&]()
			{
				bool valueChanged = false;

				if (!soundEffectManager.IsAsyncLoaded())
				{
					Gui::PushItemDisabledAndTextColor();
					Gui::SetNextItemWidth(Gui::GetContentRegionAvail().x);
					if (Gui::BeginCombo("##DummyCombo", "Loading...", ImGuiComboFlags_None))
						Gui::EndCombo();
					Gui::PopItemDisabledAndTextColor();
					return valueChanged;
				}

				constexpr u32 buttonIgnoreID = 0;
				if (btnSfxDBType == Database::GmBtnSfxType::Button && inOutID == buttonIgnoreID)
					inOutID = 1;

				const auto& sortedEntries = soundEffectManager.ViewSortedBtnSfxDB(btnSfxDBType);
				const auto* previewEntry = FindIfOrNull(sortedEntries, [&](auto* e) { return (e->ID == inOutID); });

				Gui::SetNextItemWidth(Gui::GetContentRegionAvail().x);
				const bool comboBoxOpen = Gui::BeginCombo(GuiProperty::Detail::DummyLabel, entryToCStr((previewEntry != nullptr) ? *previewEntry : nullptr), ImGuiComboFlags_HeightLarge);
				if (Gui::IsItemHovered() && Gui::IsMouseClicked(ImGuiMouseButton_Right) && previewEntry != nullptr)
					previewButtonSound(**previewEntry);

				if (comboBoxOpen)
				{
					if (Gui::Selectable(entryToCStr(nullptr), (inOutID == -1)))
					{
						inOutID = -1;
						valueChanged = true;
					}

					constexpr const char* previewButtonLabel = ICON_FA_VOLUME_UP;
					const f32 previewButtonWidth = Gui::CalcTextSize(previewButtonLabel).x;

					for (const auto* entry : sortedEntries)
					{
						if (btnSfxDBType == Database::GmBtnSfxType::Button && entry->ID == buttonIgnoreID)
							continue;

						Gui::PushID(entry);

						const bool isSelected = (entry->ID == inOutID);
						if (Gui::Selectable(entryToCStr(entry), isSelected, ImGuiSelectableFlags_AllowItemOverlap))
						{
							inOutID = entry->ID;
							valueChanged = true;
						}

						const auto previewButtonColor = Gui::GetStyleColorVec4(Gui::IsItemHovered() ? ImGuiCol_HeaderHovered : isSelected ? ImGuiCol_FrameBg : ImGuiCol_PopupBg);

						Gui::SameLine(Gui::GetContentRegionAvail().x - previewButtonWidth, 0.0f);
						Gui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, vec2(0.5f, 0.0f));
						Gui::PushStyleColor(ImGuiCol_Header, previewButtonColor);
						{
							// NOTE: Always draw as selected to avoid text clipping from rendering an otherwise transparent overlapping selectable
							if (Gui::Selectable(previewButtonLabel, true, ImGuiSelectableFlags_AllowItemOverlap | ImGuiSelectableFlags_DontClosePopups))
								previewButtonSound(*entry);
						}
						Gui::PopStyleColor(1);
						Gui::PopStyleVar();

						if (isSelected)
							Gui::SetItemDefaultFocus();
						Gui::PopID();
					}
					Gui::EndCombo();
				}

				return valueChanged;
			});
		}

		bool GuiPropertyImageFileName(std::string_view label, const char* helpText, std::string& inOutPath, AsyncLoadedImageFile& asyncImage, std::string_view basePath, ChartEditor& chartEditor)
		{
			return GuiProperty::PropertyLabelValueFunc(label, [&]
			{
				const auto& style = Gui::GetStyle();
				const auto buttonSize = Gui::GetFrameHeight();

				bool changesMade = false;

				const bool isLoading = asyncImage.IsAsyncLoading();
				Gui::PushItemDisabledAndTextColorIf(isLoading);

				Gui::PushItemWidth(Max(1.0f, (Gui::GetContentRegionAvail().x - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));

				if (isLoading)
				{
					static const std::string readOnlyPath;
					Gui::PathInputTextWithHint(GuiProperty::Detail::DummyLabel, "Loading...", const_cast<std::string*>(&readOnlyPath), ImGuiInputTextFlags_ReadOnly);
				}
				else if (Gui::PathInputTextWithHint(GuiProperty::Detail::DummyLabel, helpText, &inOutPath, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					inOutPath = IO::Path::TryMakeRelative(inOutPath, basePath);
					asyncImage.TryLoad(inOutPath, basePath);
					changesMade = true;
				}

				Gui::PopItemWidth();

				Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.y));
				Gui::SameLine(0, style.ItemInnerSpacing.x);
				if (Gui::Button("...", vec2(buttonSize)))
				{
					const auto fileDialogFilePath = chartEditor.GetOpenReadImageFileDialogPath();
					if (!fileDialogFilePath.empty())
					{
						inOutPath = IO::Path::TryMakeRelative(fileDialogFilePath, basePath);
						asyncImage.TryLoad(inOutPath, basePath);
						changesMade = true;
					}
				}
				Gui::PopStyleVar();

				if (Gui::IsItemHovered())
				{
					if (const auto image = asyncImage.GetTexSprView(); image)
					{
						// TODO: Replace with fixed position tooltip so that it doesn't move with the mouse (?)
						// TODO: Add subtle fade in and out animations (?)
						const auto fixedRect = Gui::FitFixedAspectRatioImage(ImRect(0.0f, 0.0f, 320.0f, 320.0f), image.Tex->GetSize());
						Gui::BeginTooltip();
						Gui::Image(*image.Tex, fixedRect.GetSize(), Gui::UV0_R, Gui::UV1_R);
						Gui::EndTooltip();
					}
				}

				Gui::PopItemDisabledAndTextColorIf(isLoading);

				return changesMade;
			});
		}
	}

	ChartPropertiesWindow::ChartPropertiesWindow(ChartEditor& parent, Undo::UndoManager& undoManager) : chartEditor(parent), undoManager(undoManager)
	{
	}

	void ChartPropertiesWindow::Gui(Chart& chart)
	{
		static const std::string readOnlyPath;
		constexpr const char* defaultHint = "n/a";

		GuiPropertyRAII::PropertyValueColumns columns;
		GuiProperty::TreeNode("Chart Properties", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			// NOTE: Operate on properties directly without undo actions because they all refer to the same chart object and are typically only set once. 
			//		 Accidentally undoing them while placing targets for example is never desierable and text boxes already have an internal undo stack implemented

			bool changesMade = false;

			changesMade |= GuiProperty::PropertyLabelValueFunc("Song File Name", [&]
			{
				const auto& style = Gui::GetStyle();
				const f32 buttonSize = Gui::GetFrameHeight();

				const bool songIsLoading = chartEditor.IsSongAsyncLoading();
				Gui::PushItemDisabledAndTextColorIf(songIsLoading);

				Gui::PushItemWidth(Max(1.0f, (Gui::GetContentRegionAvail().x - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));

				if (songIsLoading)
				{
					Gui::PathInputTextWithHint(GuiProperty::Detail::DummyLabel, "Loading...", const_cast<std::string*>(&readOnlyPath), ImGuiInputTextFlags_ReadOnly);
				}
				else if (Gui::PathInputTextWithHint(GuiProperty::Detail::DummyLabel, "song.ogg", &chart.SongFileName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					chartEditor.LoadSongAsync(chart.SongFileName);
					changesMade = true;
				}

				Gui::PopItemWidth();

				Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.y));
				Gui::SameLine(0, style.ItemInnerSpacing.x);
				if (Gui::Button("...", vec2(buttonSize)))
				{
					if (chartEditor.OpenLoadAudioFileDialog())
						changesMade = true;
				}
				Gui::PopStyleVar();

				Gui::PopItemDisabledAndTextColorIf(songIsLoading);
				return false;
			});

			changesMade |= GuiProperty::PropertyLabelValueFunc("Movie File Name", [&]
			{
				const auto& style = Gui::GetStyle();
				const f32 buttonSize = Gui::GetFrameHeight();

				const bool movieIsLoading = chartEditor.IsMovieAsyncLoading();
				Gui::PushItemDisabledAndTextColorIf(movieIsLoading);

				Gui::PushItemWidth(Max(1.0f, (Gui::GetContentRegionAvail().x - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));

				if (movieIsLoading)
				{
					Gui::PathInputTextWithHint(GuiProperty::Detail::DummyLabel, "Loading...", const_cast<std::string*>(&readOnlyPath), ImGuiInputTextFlags_ReadOnly);
				}
				else if (Gui::PathInputTextWithHint(GuiProperty::Detail::DummyLabel, "movie.mp4", &chart.MovieFileName, ImGuiInputTextFlags_EnterReturnsTrue))
				{
					chartEditor.LoadMovieAsync(chart.MovieFileName);
					changesMade = true;
				}

				Gui::PopItemWidth();

				Gui::PushStyleVar(ImGuiStyleVar_FramePadding, vec2(style.FramePadding.y));
				Gui::SameLine(0, style.ItemInnerSpacing.x);
				if (Gui::Button("...", vec2(buttonSize)))
				{
					if (chartEditor.OpenLoadMovieFileDialog())
						changesMade = true;
				}
				Gui::PopStyleVar();

				Gui::PopItemDisabledAndTextColorIf(movieIsLoading);
				return false;
			});

			GuiProperty::TreeNode("Song", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::InputWithHint("Title", defaultHint, chart.Properties.Song.Title);
				changesMade |= GuiProperty::InputWithHint("Artist", defaultHint, chart.Properties.Song.Artist);
				changesMade |= GuiProperty::InputWithHint("Album", defaultHint, chart.Properties.Song.Album);
				changesMade |= GuiProperty::InputWithHint("Lyricist", defaultHint, chart.Properties.Song.Lyricist);
				changesMade |= GuiProperty::InputWithHint("Arranger", defaultHint, chart.Properties.Song.Arranger);
			});

			GuiProperty::TreeNode("Extra Info", ImGuiTreeNodeFlags_None, [&]
			{
				auto& extraInfo = chart.Properties.Song.ExtraInfo;
				for (size_t i = 0; i < extraInfo.size(); i++)
				{
					GuiPropertyRAII::ID id(&extraInfo[i]);
					changesMade |= GuiProperty::PropertyFuncValueFunc(
						[&] { GuiPropertyRAII::ItemWidth width(-1.0f); return Gui::InputTextWithHint("##Key", "Property", &extraInfo[i].Key); },
						[&] { GuiPropertyRAII::ItemWidth width(-1.0f); return Gui::InputTextWithHint("##Value", defaultHint, &extraInfo[i].Value); });
				}
			});

			GuiProperty::TreeNode("Chart Creator", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::InputWithHint("Name", defaultHint, chart.Properties.Creator.Name);
				changesMade |= GuiProperty::InputMultiline("Comment", chart.Properties.Creator.Comment, vec2(0.0f, 66.0f));
			});

			GuiProperty::TreeNode("Song Images", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiPropertyImageFileName("Cover File Name", "song_jk.png", chart.Properties.Image.CoverFileName, chart.Properties.Image.Cover, chart.ChartFilePath, chartEditor);
				changesMade |= GuiPropertyImageFileName("Logo File Name", "song_logo.png", chart.Properties.Image.LogoFileName, chart.Properties.Image.Logo, chart.ChartFilePath, chartEditor);
				changesMade |= GuiPropertyImageFileName("Background File Name", "song_bg.png", chart.Properties.Image.BackgroundFileName, chart.Properties.Image.Background, chart.ChartFilePath, chartEditor);
			});

			GuiProperty::TreeNode("Song Preview", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::PropertyLabelValueFunc("Start and Duration", [&]
				{
					const auto& style = Gui::GetStyle();
					char dummyBuffer[1] = { '\0' };

					Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
					Gui::PushItemWidth((Gui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2.0f);

					auto timeWidget = [](const char* label, TimeSpan& inOutTime, TimeSpan min, TimeSpan max)
					{
						if (Gui::InputFormattedTimeSpan(label, &inOutTime, {}, ImGuiInputTextFlags_AutoSelectAll))
							inOutTime = Clamp(inOutTime, min, max);
					};

					auto songDuration = chartEditor.GetSongVoice().GetDuration();
					if (songDuration <= TimeSpan::Zero())
						songDuration = chart.DurationOrDefault();

					timeWidget("##PreviewStart", chart.Properties.SongPreview.StartTime, TimeSpan::Zero(), songDuration);
					Gui::SameLine();
					timeWidget("##PreviewDuration", chart.Properties.SongPreview.Duration, TimeSpan::Zero(), songDuration);

					Gui::PopItemWidth();
					Gui::PopStyleVar();

					return false;
				});

				GuiProperty::PropertyLabelValueFunc("Set from Cursor", [&]
				{
					const auto& style = Gui::GetStyle();
					Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
					const f32 buttonWidth = (Gui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2.0f;

					auto setTimeButton = [&](const char* label, i32 index)
					{
						if (Gui::Button(label, vec2(buttonWidth, 0.0f)))
						{
							changesMade |= true;

							const auto songPosition = chartEditor.GetSongVoice().GetPosition();
							auto songDuration = chartEditor.GetSongVoice().GetDuration();
							if (songDuration <= TimeSpan::Zero())
								songDuration = chart.DurationOrDefault();

							if (index == 0)
								chart.Properties.SongPreview.StartTime = Clamp(songPosition, TimeSpan::Zero(), songDuration);
							else
								chart.Properties.SongPreview.Duration = Clamp(songPosition - chart.Properties.SongPreview.StartTime, TimeSpan::Zero(), songDuration);
						}
					};

					setTimeButton("Set Start", 0);
					Gui::SameLine();
					setTimeButton("Set End", 1);

					Gui::PopStyleVar();
					return false;
				});

				const auto previewTimeStart = chart.Properties.SongPreview.StartTime;
				const auto previewTimeEnd = (chart.Properties.SongPreview.Duration <= TimeSpan::Zero()) ? chart.DurationOrDefault() : (previewTimeStart + chart.Properties.SongPreview.Duration);

				GuiProperty::PropertyLabelValueFunc("Listen to Preview", [&]
				{
					if (Gui::Button(isBeingPreviewed ? "Stop Preview" : "Start Preview", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
					{
						isBeingPreviewed ^= true;

						if (isBeingPreviewed)
							Audio::AudioEngine::GetInstance().EnsureStreamRunning();

						if (!previewVoiceHasBeenAdded)
						{
							previewVoiceHasBeenAdded = true;
							previewVoice = Audio::AudioEngine::GetInstance().AddVoice(Audio::SourceHandle::Invalid, "ChartProperties PreviewTimePreview", false, 1.0f, true);
						}

						previewVoice.SetSource(chartEditor.GetSongSource());
						previewVoice.SetPosition(previewTimeStart);
						previewVoice.SetIsPlaying(isBeingPreviewed);
						previewVoice.SetVolumeMap(previewTimeStart, previewTimeStart + SongPreviewFadeInDuration, 0.0f, 1.0f);
					}

					if (previewVoiceHasBeenAdded && isBeingPreviewed)
					{
						if (previewVoice.GetPosition() > previewTimeEnd)
							previewVoice.SetVolumeMap(previewTimeEnd, previewTimeEnd + SongPreviewFadeOutDuration, 1.0f, 0.0f);

						if (previewVoice.GetPosition() > (previewTimeEnd + SongPreviewFadeOutDuration + SongPreviewLoopDelay))
						{
							previewVoice.SetVolumeMap(previewTimeStart, previewTimeStart + SongPreviewFadeInDuration, 0.0f, 1.0f);
							previewVoice.SetPosition(previewTimeStart);
						}
					}

					return false;
				});

				GuiProperty::PropertyLabelValueFunc("", [&]
				{
					GuiPropertyRAII::ItemWidth width(-1.0f);

					if (isBeingPreviewed && previewVoiceHasBeenAdded)
					{
						f64 sliderSec = previewVoice.GetPosition().TotalSeconds();
						const f64 sliderSecMin = previewTimeStart.TotalSeconds();
						const f64 sliderSecMax = previewTimeEnd.TotalSeconds();

						const bool sliderUsed = Gui::SliderScalar("##PreviewProgressSlider", ImGuiDataType_Double, &sliderSec, &sliderSecMin, &sliderSecMax, previewVoice.GetPosition().FormatTime().data());
						previewSliderActiveLastFrame = previewSliderActiveThisFrame;
						previewSliderActiveThisFrame = Gui::IsItemActive();

						if (!previewSliderActiveLastFrame && previewSliderActiveThisFrame)
							previewVoice.SetIsPlaying(false);
						else if (previewSliderActiveLastFrame && !previewSliderActiveThisFrame)
							previewVoice.SetIsPlaying(true);

						if (sliderUsed)
							previewVoice.SetPosition(TimeSpan::FromSeconds(sliderSec));
					}
					else
					{
						Gui::PushItemDisabledAndTextColor();
						f32 dummyProgress = 0.0f;
						Gui::SliderFloat("##PreviewProgressSlider", &dummyProgress, 0.0f, 1.0f, "Paused");
						Gui::PopItemDisabledAndTextColor();
					}

					return false;
				});
			});

			GuiProperty::TreeNode("Difficulty", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::Combo("Type", chart.Properties.Difficulty.Type, DifficultyNames);
				changesMade |= GuiProperty::Combo("Level", chart.Properties.Difficulty.Level, DifficultyLevelNames, ImGuiComboFlags_HeightLarge, static_cast<i32>(DifficultyLevel::StarMin), static_cast<i32>(DifficultyLevel::StarMax) + 1);
			});

			// NOTE: Don't open by default to avoid loading the full font character set for as long as possible
			GuiProperty::TreeNode("Button Sounds", ImGuiTreeNodeFlags_None, [&]
			{
				auto& soundEffectManager = chartEditor.GetSoundEffectManager();
				changesMade |= GuiPropertyButtonSoundCombo("Button", chart.Properties.ButtonSound.ButtonID, Database::GmBtnSfxType::Button, soundEffectManager, lastChainSlidePreviewStopwatch);
				changesMade |= GuiPropertyButtonSoundCombo("Slide", chart.Properties.ButtonSound.SlideID, Database::GmBtnSfxType::Slide, soundEffectManager, lastChainSlidePreviewStopwatch);
				changesMade |= GuiPropertyButtonSoundCombo("Chain Slide", chart.Properties.ButtonSound.ChainSlideID, Database::GmBtnSfxType::ChainSlide, soundEffectManager, lastChainSlidePreviewStopwatch);
				changesMade |= GuiPropertyButtonSoundCombo("Slider Touch", chart.Properties.ButtonSound.SliderTouchID, Database::GmBtnSfxType::SliderTouch, soundEffectManager, lastChainSlidePreviewStopwatch);
			});

			if (changesMade)
				undoManager.SetChangesWereMade();
		});
	}
}
