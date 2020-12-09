#include "ChartPropertiesWindow.h"
#include "ChartEditor.h"
#include "ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "IO/Path.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		bool GuiPropertyButtonSoundCombo(std::string_view label, u32& inOutID, Database::GmBtnSfxType btnSfxDBType, SoundEffectManager& soundEffectManager)
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
					constexpr auto fadeOutDuration = TimeSpan::FromMilliseconds(40.0);
					constexpr auto startDuration = TimeSpan::FromSeconds(1.0);
					const auto[firstSource, subSource, successSource, failureSource] = soundEffectManager.GetChainSlideSound(entry.ID);

					Audio::Voice startVoice = engine.AddVoice(firstSource, formatVoiceName(entry.Chain.SfxNameFirst), true);
					startVoice.SetRemoveOnEnd(true);
					startVoice.SetVolumeMap(startDuration, startDuration + fadeOutDuration, 1.0f, 0.0f);

					Audio::Voice endVoice = engine.AddVoice(successSource, formatVoiceName(entry.Chain.SfxNameSuccess), true);
					endVoice.SetRemoveOnEnd(true);
					endVoice.SetPosition(-startDuration);
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
					if (Gui::InternalVariableWidthBeginCombo("##DummyCombo", "Loading...", ImGuiComboFlags_None, Gui::GetContentRegionAvailWidth()))
						Gui::EndCombo();
					Gui::PopItemDisabledAndTextColor();
					return valueChanged;
				}

				constexpr u32 buttonIgnoreID = 0;
				if (btnSfxDBType == Database::GmBtnSfxType::Button && inOutID == buttonIgnoreID)
					inOutID = 1;

				const auto& sortedEntries = soundEffectManager.ViewSortedBtnSfxDB(btnSfxDBType);
				const auto* previewEntry = FindIfOrNull(sortedEntries, [&](auto* e) { return (e->ID == inOutID); });

				const bool comboBoxOpen = Gui::InternalVariableWidthBeginCombo(GuiProperty::Detail::DummyLabel, entryToCStr((previewEntry != nullptr) ? *previewEntry : nullptr), ImGuiComboFlags_HeightLarge, Gui::GetContentRegionAvailWidth());
				if (Gui::IsItemHovered() && Gui::IsMouseClicked(1) && previewEntry != nullptr)
					previewButtonSound(**previewEntry);

				if (comboBoxOpen)
				{
					if (Gui::Selectable(entryToCStr(nullptr), (inOutID == -1)))
					{
						inOutID = -1;
						valueChanged = true;
					}

					for (const auto* entry : sortedEntries)
					{
						if (btnSfxDBType == Database::GmBtnSfxType::Button && entry->ID == buttonIgnoreID)
							continue;

						const bool isSelected = (entry->ID == inOutID);
						if (Gui::Selectable(entryToCStr(entry), isSelected))
						{
							inOutID = entry->ID;
							valueChanged = true;
						}

						if (Gui::IsItemHovered() && Gui::IsMouseClicked(1))
							previewButtonSound(*entry);

						if (isSelected)
							Gui::SetItemDefaultFocus();
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

				Gui::PushItemWidth(std::max(1.0f, (Gui::GetContentRegionAvailWidth() - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));

				if (isLoading)
				{
					char readOnlyBuffer[1] = { '\0' };
					Gui::InputTextWithHint(GuiProperty::Detail::DummyLabel, "Loading...", readOnlyBuffer, sizeof(readOnlyBuffer), ImGuiInputTextFlags_ReadOnly);
				}
				else if (Gui::InputTextWithHint(GuiProperty::Detail::DummyLabel, helpText, &inOutPath, ImGuiInputTextFlags_EnterReturnsTrue))
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
				const auto buttonSize = Gui::GetFrameHeight();

				const bool songIsLoading = chartEditor.IsSongAsyncLoading();
				Gui::PushItemDisabledAndTextColorIf(songIsLoading);

				Gui::PushItemWidth(std::max(1.0f, (Gui::GetContentRegionAvailWidth() - 1.0f) - (buttonSize + style.ItemInnerSpacing.x)));

				if (songIsLoading)
				{
					char readOnlyBuffer[1] = { '\0' };
					Gui::InputTextWithHint(GuiProperty::Detail::DummyLabel, "Loading...", readOnlyBuffer, sizeof(readOnlyBuffer), ImGuiInputTextFlags_ReadOnly);
				}
				else if (Gui::InputTextWithHint(GuiProperty::Detail::DummyLabel, "song.ogg", &chart.SongFileName, ImGuiInputTextFlags_EnterReturnsTrue))
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

			GuiProperty::TreeNode("Difficulty", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				changesMade |= GuiProperty::Combo("Type", chart.Properties.Difficulty.Type, DifficultyNames);
				changesMade |= GuiProperty::Combo("Level", chart.Properties.Difficulty.Level, DifficultyLevelNames, ImGuiComboFlags_HeightLarge, static_cast<i32>(DifficultyLevel::StarMin), static_cast<i32>(DifficultyLevel::StarMax) + 1);
			});

			// NOTE: Don't open by default to avoid loading the full font character set for as long as possible
			GuiProperty::TreeNode("Button Sounds", ImGuiTreeNodeFlags_None, [&]
			{
				auto& soundEffectManager = chartEditor.GetSoundEffectManager();
				changesMade |= GuiPropertyButtonSoundCombo("Button", chart.Properties.ButtonSound.ButtonID, Database::GmBtnSfxType::Button, soundEffectManager);
				changesMade |= GuiPropertyButtonSoundCombo("Slide", chart.Properties.ButtonSound.SlideID, Database::GmBtnSfxType::Slide, soundEffectManager);
				changesMade |= GuiPropertyButtonSoundCombo("Chain Slide", chart.Properties.ButtonSound.ChainSlideID, Database::GmBtnSfxType::ChainSlide, soundEffectManager);
				changesMade |= GuiPropertyButtonSoundCombo("Slider Touch", chart.Properties.ButtonSound.SliderTouchID, Database::GmBtnSfxType::SliderTouch, soundEffectManager);
			});

			if (changesMade)
				undoManager.SetChangesWereMade();
		});
	}
}
