#include "ChartEditor.h"
#include "ChartCommands.h"
#include "FileFormat/PJEFile.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "IO/Directory.h"
#include "Core/ComfyStudioApplication.h"
#include "Core/ComfyStudioSettings.h"
#include "System/ComfyData.h"
#include "Misc/StringUtil.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr std::string_view FallbackChartFileName = "Untitled Chart.csfm";

		std::string_view GetChartSaveDialogFileName(const Chart& chart)
		{
			if (!chart.ChartFilePath.empty())
				return IO::Path::GetFileName(chart.ChartFilePath, false);

			if (!chart.Properties.Song.Title.empty())
				return chart.Properties.Song.Title;

			return IO::Path::TrimExtension(FallbackChartFileName);
		}
	}

	ChartEditor::ChartEditor(ComfyStudioApplication& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		chart = std::make_unique<Chart>();
		chart->UpdateMapTimes();
		chart->Properties.Creator.Name = GlobalUserData.ChartProperties.ChartCreatorDefaultName;

		songVoice = Audio::AudioEngine::GetInstance().AddVoice(Audio::SourceHandle::Invalid, "ChartEditor SongVoice", false, 1.0f, true);
		buttonSoundController = std::make_unique<ButtonSoundController>(soundEffectManager);

		renderer = std::make_unique<Render::Renderer2D>();
		timeline = std::make_unique<TargetTimeline>(*this, undoManager, *buttonSoundController);

		renderWindow = std::make_unique<TargetRenderWindow>(*this, *timeline, undoManager, *renderer);
		renderWindow->RegisterRenderCallback([this](auto& renderWindow, auto& renderer) { presetWindow.OnRenderWindowRender(*chart, renderWindow, renderer); });
		renderWindow->RegisterOverlayGuiCallback([this](auto& renderWindow, auto& drawList) { presetWindow.OnRenderWindowOverlayGui(*chart, renderWindow, drawList); });

		// TODO: Load async (?)
		editorSprites = System::Data.Load<Graphics::SprSet>(System::Data.FindFile("sprite/spr_chart_editor.bin"));
		timeline->OnEditorSpritesLoaded(editorSprites.get());
		presetWindow.OnEditorSpritesLoaded(editorSprites.get());

		SyncWorkingChartPointers();

		const auto fileToOpen = parentApplication.GetFileToOpenOnStartup();
		if (!fileToOpen.empty() && Util::MatchesInsensitive(IO::Path::GetExtension(fileToOpen), ComfyStudioChartFile::Extension))
			LoadNativeChartFileSync(fileToOpen);

#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
		unixTimeOnChartBegin = Discord::GlobalGetCurrentUnixTime();
#endif
	}

	const char* ChartEditor::GetName() const
	{
		return "Chart Editor";
	}

	ImGuiWindowFlags ChartEditor::GetFlags() const
	{
		return BaseWindow::NoWindowFlags;
	}

	void ChartEditor::Gui()
	{
		Gui::GetCurrentWindow()->Hidden = true;

		const auto& buttonIDs = chart->Properties.ButtonSound;
		buttonSoundController->SetIDs(buttonIDs.ButtonID, buttonIDs.SlideID, buttonIDs.ChainSlideID, buttonIDs.SliderTouchID);
		buttonSoundController->SetMasterVolume(GlobalUserData.System.Audio.ButtonSoundVolume);

		songVoice.SetVolume(GlobalUserData.System.Audio.SongVolume);

		UpdateApplicationClosingRequest();
		UpdateGlobalControlInput();
		UpdateApplicationWindowTitle();
		UpdateDiscordStatusIfEnabled(false);
		UpdateAsyncSongSourceLoading();

		GuiChildWindows();

		GuiSettingsPopup();
		GuiPVScriptImportPopup();
		GuiPVScriptExportPopup();
		GuiFileNotFoundPopup();
		GuiSaveConfirmationPopup();

		undoManager.FlushExecuteEndOfFrameCommands();
	}

	void ChartEditor::GuiMenu()
	{
		if (Gui::BeginMenu("File"))
		{
			if (Gui::MenuItem("New Chart", Input::ToString(GlobalUserData.Input.ChartEditor_ChartNew).data(), false, true))
				CheckOpenSaveConfirmationPopupThenCall([this] { CreateNewChart(); });

			if (Gui::MenuItem("Open...", Input::ToString(GlobalUserData.Input.ChartEditor_ChartOpen).data(), false, true))
				CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadNativeChartFileDialog(); });

			const auto& recentFilesView = GlobalAppData.RecentFiles.ChartFiles.View();
			if (Gui::BeginMenu("Open Recent", !recentFilesView.empty()))
			{
				size_t recentFileIndex = 0;
				std::for_each(recentFilesView.rbegin(), recentFilesView.rend(), [&](const auto& path)
				{
					// NOTE: Even without handling keyboard input this still helps with readability
					const char shortcutBuffer[2] = { (recentFileIndex < 9) ? static_cast<char>('1' + recentFileIndex) : '\0', '\0' };

					if (Gui::MenuItem(path.c_str(), shortcutBuffer))
					{
						if (IO::File::Exists(path))
						{
							CheckOpenSaveConfirmationPopupThenCall([this, path] { LoadNativeChartFileSync(path); });
						}
						else
						{
							fileNotFoundPopup.OpenOnNextFrame = true;
							fileNotFoundPopup.NotFoundPath = path;
							fileNotFoundPopup.QuestionToTheUser = "Remove from Recent Files list?";
							fileNotFoundPopup.OnYesClickedFunction = [this]()
							{
								GlobalAppData.RecentFiles.ChartFiles.Remove(fileNotFoundPopup.NotFoundPath);
							};
						}
					}
					recentFileIndex++;
				});

				Gui::Separator();
				if (Gui::MenuItem("Clear Items"))
					GlobalAppData.RecentFiles.ChartFiles.Clear();

				Gui::EndMenu();
			}

			const auto chartDirectory = IO::Path::GetDirectoryName(chart->ChartFilePath);
			if (Gui::MenuItem("Open Chart Directory...", Input::ToString(GlobalUserData.Input.ChartEditor_ChartOpenDirectory).data(), false, !chartDirectory.empty()))
				OpenChartDirectoryInExplorer();

			Gui::Separator();

			if (Gui::MenuItem("Save", Input::ToString(GlobalUserData.Input.ChartEditor_ChartSave).data(), false, true))
				TrySaveNativeChartFileOrOpenDialog();
			if (Gui::MenuItem("Save As...", Input::ToString(GlobalUserData.Input.ChartEditor_ChartSaveAs).data(), false, true))
				OpenSaveNativeChartFileDialog();
			Gui::Separator();

			if (Gui::BeginMenu("Import"))
			{
				if (Gui::MenuItem("Import UPDC Chart...", nullptr, false, true))
					CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadImportPJEChartFileDialog(); });

				if (Gui::MenuItem("Import PV Script Chart...", nullptr, false, true))
					CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadImportPVScriptFileDialogThenOpenImportWindow(); });

				Gui::EndMenu();
			}

			if (Gui::BeginMenu("Export"))
			{
				if (Gui::MenuItem("Export UPDC Chart...", nullptr, false, true))
					OpenSaveExportPJEChartFileDialog();

				if (Gui::MenuItem("Expot PV Script MData...", nullptr, false, true))
					OpenPVScriptExportWindow();

				if (Gui::MenuItem("Expot PV Script Chart...", nullptr, false, true))
					OpenSaveExportSimplePVScriptChartFileDialog();

				Gui::EndMenu();
			}

			Gui::Separator();

			if (Gui::MenuItem("Exit", Input::ToString(Input::Binding(Input::KeyCode_F4, Input::KeyModifiers_Alt)).data()))
				applicationExitRequested = true;

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Edit"))
		{
			if (Gui::MenuItem("Undo", Input::ToString(GlobalUserData.Input.ChartEditor_Undo).data(), false, undoManager.CanUndo()))
				undoManager.Undo();
			if (Gui::MenuItem("Redo", Input::ToString(GlobalUserData.Input.ChartEditor_Redo).data(), false, undoManager.CanRedo()))
				undoManager.Redo();

			Gui::Separator();
			if (Gui::MenuItem("Settings...", Input::ToString(GlobalUserData.Input.ChartEditor_OpenSettings).data(), false, true))
				settingsPopup.OpenOnNextFrame = true;

			Gui::EndMenu();
		}

		constexpr bool debugPopupPlaytestWindowEnabled = false;
		static bool debugPopupPlaytestWindowOpen = false;

		if constexpr (debugPopupPlaytestWindowEnabled)
		{
			if (debugPopupPlaytestWindowOpen)
			{
				if (Gui::Begin("Popout Playtest Window (Debug)", &debugPopupPlaytestWindowOpen, (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking)))
					OnExclusiveGui();
				Gui::End();
			}
		}

		if (Gui::BeginMenu("Playtest"))
		{
			if (Gui::MenuItem("Start from Beginning", Input::ToString(GlobalUserData.Input.ChartEditor_StartPlaytestFromStart).data()))
				StartPlaytesting(false);

			if (Gui::MenuItem("Start from Cursor", Input::ToString(GlobalUserData.Input.ChartEditor_StartPlaytestFromCursor).data()))
				StartPlaytesting(true);

			Gui::Separator();
			bool autoplayEnabled = (playTestWindow == nullptr) ? false : playTestWindow->GetAutoplayEnabled();
			if (Gui::MenuItemDontClosePopup("Autoplay Enabled", nullptr, &autoplayEnabled))
				GetOrCreatePlayTestWindow().SetAutoplayEnabled(autoplayEnabled);

			if constexpr (debugPopupPlaytestWindowEnabled)
				Gui::MenuItemDontClosePopup("Popout Playtest Window (Debug)", nullptr, &debugPopupPlaytestWindowOpen);

			Gui::EndMenu();
		}
	}

	void ChartEditor::OnEditorComponentMadeActive()
	{
		lastSetWindowTitle.clear();
	}

	ApplicationHostCloseResponse ChartEditor::OnApplicationClosing()
	{
		if (settingsPopup.WasOpenLastFrame)
			settingsPopup.Window.OnCloseButtonClicked();

		if (undoManager.GetHasPendingChanged())
		{
			if (parentApplication.GetExclusiveFullscreenGui())
				StopPlaytesting(PlayTestExitType::ReturnCurrentTime);

			applicationExitRequested = true;
			return ApplicationHostCloseResponse::SupressExit;
		}
		else
		{
			return ApplicationHostCloseResponse::Exit;
		}
	}

	void ChartEditor::OnExclusiveGui()
	{
		UpdateDiscordStatusIfEnabled(true);

		auto& playTestWindow = GetOrCreatePlayTestWindow();
		playTestWindow.ExclusiveGui();

		if (const auto exitType = playTestWindow.GetAndClearExitRequestThisFrame(); exitType != PlayTestExitType::None)
			StopPlaytesting(exitType);
	}

	bool ChartEditor::OpenLoadAudioFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Open Audio File";
		fileDialog.FileName;
		fileDialog.DefaultExtension;
		fileDialog.Filters =
		{
			{ "Audio Files (*.flac;*.ogg;*.mp3;*.wav)", "*.flac;*.ogg;*.mp3;*.wav" },
			{ "FLAC Files (*.flac)", "*.flac" },
			{ "Ogg Vorbis (*.ogg)", "*.ogg" },
			{ "MP3 Files (*.mp3)", "*.mp3" },
			{ "WAV Files (*.wav)", "*.wav" },
			{ std::string(IO::Shell::FileDialog::AllFilesFilterName), std::string(IO::Shell::FileDialog::AllFilesFilterSpec) },
		};
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		LoadSongAsync(IO::Path::Normalize(fileDialog.OutFilePath));
		return true;
	}

	bool ChartEditor::OnFileDropped(std::string_view filePath)
	{
		const auto extension = IO::Path::GetExtension(filePath);

		if (IO::Path::DoesAnyPackedExtensionMatch(extension, PVScript::Extension))
		{
			CheckOpenSaveConfirmationPopupThenCall([this, path = std::string(filePath)]() { OpenPVScriptImportWindow(path); });
			return true;
		}

		if (IO::Path::DoesAnyPackedExtensionMatch(extension, ComfyStudioChartFile::Extension))
		{
			CheckOpenSaveConfirmationPopupThenCall([this, path = std::string(filePath)] { LoadNativeChartFileSync(path); });
			return true;
		}

		if (IO::Path::DoesAnyPackedExtensionMatch(extension, Legacy::PJEFile::Extension))
		{
			CheckOpenSaveConfirmationPopupThenCall([this, path = std::string(filePath)] { ImportPJEChartFileSync(path); });
			return true;
		}

		if (IO::Path::DoesAnyPackedExtensionMatch(extension, ".flac;.ogg;.mp3;.wav"))
		{
			LoadSongAsync(filePath);
			undoManager.SetChangesWereMade();
			return true;
		}

		if (IO::Path::DoesAnyPackedExtensionMatch(extension, ".png;.jpg;.jpeg;.bmp;.gif"))
		{
			auto setAndLoad = [&](std::string& outFileName, AsyncLoadedImageFile& outImage)
			{
				outImage.TryLoad(filePath);
				outFileName = IO::Path::TryMakeRelative(filePath, chart->ChartFilePath);
				undoManager.SetChangesWereMade();
				return true;
			};

			static constexpr std::array coverStrings { "cover", "jacket", "song_jk", "folder", "album" };
			static constexpr std::array logoStrings { "logo", };
			static constexpr std::array backgroundStrings { "background", "song_bg", "haikei" };

			const auto lowerCaseFileName = Util::ToLowerCopy(std::string(IO::Path::GetFileName(filePath)));
			auto isPartOfFileName = [&lowerCaseFileName](std::string_view subString) { return Util::Contains(lowerCaseFileName, subString); };

			if (std::any_of(coverStrings.begin(), coverStrings.end(), isPartOfFileName))
				return setAndLoad(chart->Properties.Image.CoverFileName, chart->Properties.Image.Cover);

			if (std::any_of(logoStrings.begin(), logoStrings.end(), isPartOfFileName))
				return setAndLoad(chart->Properties.Image.LogoFileName, chart->Properties.Image.Logo);

			if (std::any_of(backgroundStrings.begin(), backgroundStrings.end(), isPartOfFileName))
				return setAndLoad(chart->Properties.Image.BackgroundFileName, chart->Properties.Image.Background);
		}

		return false;
	}

	void ChartEditor::LoadSongAsync(std::string_view filePath)
	{
		UnloadSong();

		songSourceFilePathAbsolute = IO::Path::ResolveRelativeTo(filePath, chart->ChartFilePath);
		songSourceFuture = Audio::AudioEngine::GetInstance().LoadSourceAsync(songSourceFilePathAbsolute);

		chart->SongFileName = IO::Path::TryMakeRelative(songSourceFilePathAbsolute, chart->ChartFilePath);
	}

	void ChartEditor::UnloadSong()
	{
		// NOTE: Optimally the last action should be canceled through a stop token instead
		if (songSourceFuture.valid())
			Audio::AudioEngine::GetInstance().UnloadSource(songSourceFuture.get());

		Audio::AudioEngine::GetInstance().UnloadSource(songSource);
		songSource = Audio::SourceHandle::Invalid;
		songSourceFilePathAbsolute.clear();

		timeline->OnSongLoaded();
	}

	void ChartEditor::CreateNewChart()
	{
		undoManager.ClearAll();
		chart = std::make_unique<Chart>();
		chart->UpdateMapTimes();
		chart->Properties.Creator.Name = GlobalUserData.ChartProperties.ChartCreatorDefaultName;
		UnloadSong();

		SyncWorkingChartPointers();

		if (isPlaying)
			timeline->StopPlayback();

		timeline->SetCursorTime(TimeSpan::Zero());
		timeline->ResetScrollAndZoom();

#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
		unixTimeOnChartBegin = Discord::GlobalGetCurrentUnixTime();
#endif
	}

	void ChartEditor::LoadNativeChartFileSync(std::string_view filePath)
	{
		assert(Util::EndsWithInsensitive(filePath, ComfyStudioChartFile::Extension));
		GlobalAppData.RecentFiles.ChartFiles.Add(filePath);

		// TODO: Display error GUI if unable to load (?)
		CreateNewChart();
		const auto chartFile = IO::File::Load<ComfyStudioChartFile>(filePath);

		chart = (chartFile != nullptr) ? chartFile->MoveToChart() : std::make_unique<Chart>();
		chart->UpdateMapTimes();
		chart->ChartFilePath = std::string(filePath);

		if (!chart->SongFileName.empty())
			LoadSongAsync(chart->SongFileName);

		chart->Properties.Image.Cover.TryLoad(chart->Properties.Image.CoverFileName, chart->ChartFilePath);
		chart->Properties.Image.Logo.TryLoad(chart->Properties.Image.LogoFileName, chart->ChartFilePath);
		chart->Properties.Image.Background.TryLoad(chart->Properties.Image.BackgroundFileName, chart->ChartFilePath);

		SyncWorkingChartPointers();
	}

	void ChartEditor::SaveNativeChartFileAsync(std::string_view filePath)
	{
		if (!filePath.empty())
			chart->ChartFilePath = filePath;

		if (!chart->ChartFilePath.empty())
		{
			if (!chart->SongFileName.empty() && !songSourceFilePathAbsolute.empty())
				chart->SongFileName = IO::Path::TryMakeRelative(songSourceFilePathAbsolute, chart->ChartFilePath);

			if (chartSaveFileFuture.valid())
				chartSaveFileFuture.get();

			lastSavedChartFile = std::make_unique<ComfyStudioChartFile>(*chart);
			chartSaveFileFuture = IO::File::SaveAsync(chart->ChartFilePath, lastSavedChartFile.get());

			undoManager.ClearPendingChangesFlag();
			GlobalAppData.RecentFiles.ChartFiles.Add(chart->ChartFilePath);
		}
	}

	void ChartEditor::OpenChartDirectoryInExplorer() const
	{
		const auto chartDirectory = IO::Path::GetDirectoryName(chart->ChartFilePath);
		if (!chartDirectory.empty() && IO::Directory::Exists(chartDirectory))
			IO::Shell::OpenInExplorer(chartDirectory);
	}

	bool ChartEditor::OpenReadNativeChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Open Chart";
		fileDialog.DefaultExtension = ComfyStudioChartFile::Extension;
		fileDialog.Filters = { { std::string(ComfyStudioChartFile::FilterName), std::string(ComfyStudioChartFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		LoadNativeChartFileSync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::OpenSaveNativeChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Save Chart As";
		fileDialog.FileName = GetChartSaveDialogFileName(*chart);
		fileDialog.DefaultExtension = ComfyStudioChartFile::Extension;
		fileDialog.Filters = { { std::string(ComfyStudioChartFile::FilterName), std::string(ComfyStudioChartFile::FilterSpec) }, };

		const bool songFileIsAbsolute = (!chart->SongFileName.empty() && !IO::Path::IsRelative(chart->SongFileName));
		const bool copySongFileDefaultValue = false;

		bool copySongFile = copySongFileDefaultValue;
		fileDialog.CustomizeItems = { { IO::Shell::Custom::ItemType::Checkbox, "Copy Song to Directory", songFileIsAbsolute ? &copySongFile : nullptr } };

		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return false;

		if (copySongFile && songFileIsAbsolute)
		{
			const auto relativeSongFileName = IO::Path::GetFileName(chart->SongFileName);
			const auto newChartDirectory = IO::Path::GetDirectoryName(fileDialog.OutFilePath);
			const auto newAbsoulteSongPath = IO::Path::Combine(newChartDirectory, relativeSongFileName);

			if (IO::File::Copy(chart->SongFileName, newAbsoulteSongPath))
			{
				chart->SongFileName = relativeSongFileName;
				songSourceFilePathAbsolute = newAbsoulteSongPath;
			}
		}

		SaveNativeChartFileAsync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::TrySaveNativeChartFileOrOpenDialog()
	{
		if (chart->ChartFilePath.empty())
			return OpenSaveNativeChartFileDialog();
		else
			SaveNativeChartFileAsync();
		return true;
	}

	void ChartEditor::ImportPJEChartFileSync(std::string_view filePath)
	{
		assert(Util::EndsWithInsensitive(filePath, Legacy::PJEFile::Extension));

		CreateNewChart();
		const auto pjeFile = IO::File::Load<Legacy::PJEFile>(filePath);

		chart = (pjeFile != nullptr) ? pjeFile->ToChart() : std::make_unique<Chart>();
		chart->UpdateMapTimes();

		// NOTE: Unable to use relative file path because imported chart data don't and shouldn't set a working directory
		LoadSongAsync((pjeFile != nullptr) ? pjeFile->TryFindSongFilePath(filePath) : "");

		SyncWorkingChartPointers();
		undoManager.SetChangesWereMade();
	}

	void ChartEditor::ExportPJEChartFileSync(std::string_view filePath)
	{
		assert(Util::EndsWithInsensitive(filePath, Legacy::PJEFile::Extension));

		auto pjeFile = Legacy::PJEFile(*chart);
		IO::File::Save(filePath, pjeFile);
	}

	bool ChartEditor::OpenReadImportPJEChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Import UPDC Chart";
		fileDialog.DefaultExtension = Legacy::PJEFile::Extension;
		fileDialog.Filters = { { std::string(Legacy::PJEFile::FilterName), std::string(Legacy::PJEFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		ImportPJEChartFileSync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::OpenSaveExportPJEChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Export As UPDC Chart";
		fileDialog.FileName = GetChartSaveDialogFileName(*chart);
		fileDialog.DefaultExtension = Legacy::PJEFile::Extension;
		fileDialog.Filters = { { std::string(Legacy::PJEFile::FilterName), std::string(Legacy::PJEFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return false;

		ExportPJEChartFileSync(fileDialog.OutFilePath);
		return true;
	}

	void ChartEditor::OpenPVScriptImportWindow(std::string_view filePath)
	{
		CreateNewChart();
		pvScriptImportPopup.Window.SetScriptFilePath(filePath);
		pvScriptImportPopup.OpenOnNextFrame = true;
	}

	void ChartEditor::OpenPVScriptExportWindow()
	{
		pvScriptExportPopup.OpenOnNextFrame = true;
	}

	bool ChartEditor::OpenSaveExportSimplePVScriptChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Export As PV Script Chart";
		fileDialog.FileName = GetChartSaveDialogFileName(*chart);
		fileDialog.DefaultExtension = PVScript::Extension;
		fileDialog.Filters = { { std::string(PVScript::FilterName), std::string(PVScript::FilterSpec) }, };
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return false;

		pvScriptExportPopup.Window.ConvertAndSaveSimpleScriptSync(fileDialog.OutFilePath, *chart);
		return true;
	}

	bool ChartEditor::OpenReadImportPVScriptFileDialogThenOpenImportWindow()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Import PV Script Chart";
		fileDialog.DefaultExtension = PVScript::Extension;
		fileDialog.Filters = { { std::string(PVScript::FilterName), std::string(PVScript::FilterSpec) }, };
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		OpenPVScriptImportWindow(fileDialog.OutFilePath);
		return true;
	}

	void ChartEditor::CheckOpenSaveConfirmationPopupThenCall(std::function<void()> onSuccess)
	{
		if (undoManager.GetHasPendingChanged())
		{
			saveConfirmationPopup.OpenOnNextFrame = true;
			saveConfirmationPopup.OnSuccessFunction = std::move(onSuccess);
		}
		else
		{
			onSuccess();
		}
	}

	std::string ChartEditor::GetOpenReadImageFileDialogPath() const
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Open Image File";
		fileDialog.FileName;
		fileDialog.DefaultExtension = ".png";
		fileDialog.Filters =
		{
			{ "Image Files (*.png;*.jpg;*.jpeg;.bmp;.gif)", "*.png;*.jpg;*.jpeg;.bmp;.gif" },
			{ "PNG Files (*.png)", "*.png" },
			{ "JPEG Files (*.jpg;*.jpeg)", "*.jpg;*.jpeg" },
			{ "BMP Files (*.bmp)", "*.bmp" },
			{ "GIF Files (*.gif)", "*.gif" },
			{ std::string(IO::Shell::FileDialog::AllFilesFilterName), std::string(IO::Shell::FileDialog::AllFilesFilterSpec) },
		};
		fileDialog.ParentWindowHandle = ComfyStudioApplication::GetGlobalWindowFocusHandle();

		if (fileDialog.OpenRead())
			return std::move(fileDialog.OutFilePath);
		else
			return "";
	}

	bool ChartEditor::IsSongAsyncLoading() const
	{
		return (songSourceFuture.valid() && !songSourceFuture._Is_ready());
	}

	TimeSpan ChartEditor::GetPlaybackTimeAsync() const
	{
#if 1 // NOTE: Pottentially less precise and reliable, especially if the audio render thread is overloaded. The improved smoothness should be well worth it however
		return (songVoice.GetPositionSmooth() - chart->StartOffset);
#else
		return (songVoice.GetPosition() - chart->StartOffset);
#endif
	}

	void ChartEditor::SetPlaybackTime(TimeSpan value)
	{
		songVoice.SetPosition(value + chart->StartOffset);
	}

	TimeSpan ChartEditor::GetPlaybackTimeOnPlaybackStart() const
	{
		return playbackTimeOnPlaybackStart;
	}

	SoundEffectManager& ChartEditor::GetSoundEffectManager()
	{
		return soundEffectManager;
	}

	void ChartEditor::UpdateApplicationClosingRequest()
	{
		if (!applicationExitRequested)
			return;

		applicationExitRequested = false;
		CheckOpenSaveConfirmationPopupThenCall([this]
		{
			if (chartSaveFileFuture.valid())
				chartSaveFileFuture.get();

			parentApplication.Exit();
		});
	}

	void ChartEditor::UpdateGlobalControlInput()
	{
		// TODO: Move all keycodes into KeyBindings header and implement modifier + keycode string format helper

		// HACK: Works for now I guess...
		if (!parentApplication.GetHost().IsWindowFocused() || Gui::GetActiveID() != 0)
			return;

		// HACK: Not quite sure about this one yet but seems reasonable..?
		if (Gui::GetCurrentContext()->OpenPopupStack.Size > 0)
			return;

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_StartPlaytestFromStart, false))
			StartPlaytesting(false);

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_StartPlaytestFromCursor, false))
			StartPlaytesting(true);

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_Undo, true))
			undoManager.Undo();

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_Redo, true))
			undoManager.Redo();

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_OpenSettings, false))
			settingsPopup.OpenOnNextFrame = true;

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_ChartNew, false))
			CheckOpenSaveConfirmationPopupThenCall([this] { CreateNewChart(); });

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_ChartOpen, false))
			CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadNativeChartFileDialog(); });

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_ChartSave, false))
			TrySaveNativeChartFileOrOpenDialog();

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_ChartSaveAs, false))
			OpenSaveNativeChartFileDialog();

		if (Input::IsAnyPressed(GlobalUserData.Input.ChartEditor_ChartOpenDirectory, false))
			OpenChartDirectoryInExplorer();
	}

	void ChartEditor::UpdateApplicationWindowTitle()
	{
		windowTitle = chart->ChartFilePath.empty() ? FallbackChartFileName : IO::Path::GetFileName(chart->ChartFilePath, true);

		if (undoManager.GetHasPendingChanged())
			windowTitle += '*';

		// NOTE: Comparing a few characters each frame should be well worth not having to manually sync the window title every time the chart file path is updated externally
		if (windowTitle != lastSetWindowTitle)
		{
			parentApplication.SetFormattedWindowTitle(windowTitle);
			lastSetWindowTitle = windowTitle;
		}
	}

	void ChartEditor::UpdateDiscordStatusIfEnabled(bool isPlaytesting)
	{
#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
		if (Discord::GlobalGetIsRuntimeEnabled() && chart != nullptr)
		{
			discordStatus.UnixStartTime = GlobalUserData.System.Discord.ShareElapsedTime ? unixTimeOnChartBegin : 0;

			discordStatus.LargeImageKey = Discord::ImageKey::ComfyApplicatonIcon;
			discordStatus.LargeImageText = "Comfy Studio";
			discordStatus.SmallImageKey = Discord::ImageKey::ComfyApplicatonIcon;
			discordStatus.SmallImageText = IndexOr(static_cast<std::underlying_type_t<Difficulty>>(chart->Properties.Difficulty.Type), DifficultyNames, "Invalid");;

			discordStatus.State = (isPlaytesting) ? "Playing a Chart" : "Creating a Chart";
			discordStatus.Details.clear();
			discordStatus.Details += chart->SongTitleOrDefault();
			discordStatus.Details += " by ";
			discordStatus.Details += chart->Properties.Song.Artist.empty() ? "Unknown" : chart->Properties.Song.Artist;

			Discord::GlobalSetStatus(discordStatus);
		}
#endif
	}

	void ChartEditor::UpdateAsyncSongSourceLoading()
	{
		if (!songSourceFuture.valid() || !songSourceFuture._Is_ready())
			return;

		const auto oldPlaybackTime = GetPlaybackTimeAsync();
		const auto newSongSource = songSourceFuture.get();
		const auto oldSongSource = songSource;

		Audio::AudioEngine::GetInstance().UnloadSource(oldSongSource);
		songVoice.SetSource(newSongSource);
		songSource = newSongSource;

		if (chart->Duration <= TimeSpan::Zero())
		{
			chart->Duration = songVoice.GetDuration();
			undoManager.SetChangesWereMade();
		}

		auto& songInfo = chart->Properties.Song;
		{
			// TODO: Extract metadata from audio file
			if (songInfo.Title.empty())
			{
				songInfo.Title = IO::Path::GetFileName(songSourceFilePathAbsolute, false);
				undoManager.SetChangesWereMade();
			}

			songInfo.Artist;
			songInfo.Album;
		}

		SetPlaybackTime(oldPlaybackTime);
		timeline->OnSongLoaded();
	}

	void ChartEditor::GuiChildWindows()
	{
		if (Gui::Begin(ICON_FA_HISTORY "  Chart Editor History##ChartEditor"))
			historyWindow.Gui();
		Gui::End();

		if (Gui::Begin(ICON_FA_DRAFTING_COMPASS "  Sync Target Presets##ChartEditor", nullptr, ImGuiWindowFlags_None))
			presetWindow.SyncGui(*chart);
		Gui::End();
		if (Gui::Begin(ICON_FA_DRAFTING_COMPASS "  Sequence Target Presets##ChartEditor", nullptr, ImGuiWindowFlags_None))
			presetWindow.SequenceGui(*chart);
		Gui::End();
		presetWindow.UpdateStateAfterBothGuiPartsHaveBeenDrawn();

		if (Gui::Begin(ICON_FA_MUSIC "  Target Timeline##ChartEditor##ChartEditor", nullptr, ImGuiWindowFlags_None))
			timeline->DrawTimelineGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_STOPWATCH "  BPM Calculator##ChartEditor", nullptr, ImGuiWindowFlags_None))
			bpmCalculatorWindow.Gui(*chart, GetIsPlayback() ? GetPlaybackTimeOnPlaybackStart() : GetPlaybackTimeAsync(), timeline->GetMetronome(), *buttonSoundController);
		Gui::End();

		if (Gui::Begin(ICON_FA_SYNC "  Sync Window##ChartEditor", nullptr, ImGuiWindowFlags_None))
			syncWindow.Gui(*chart, *timeline);
		Gui::End();

		if (Gui::Begin(ICON_FA_INFO_CIRCLE "  Target Inspector##ChartEditor", nullptr, ImGuiWindowFlags_None))
			inspector.Gui(*chart);
		Gui::End();

		renderWindow->BeginEndGui(ICON_FA_CHART_BAR "  Target Preview##ChartEditor");

		if (Gui::Begin(ICON_FA_LIST_UL "  Chart Properties##ChartEditor", nullptr, ImGuiWindowFlags_None))
			chartPropertiesWindow.Gui(*chart);
		Gui::End();
	}

	void ChartEditor::GuiSettingsPopup()
	{
		constexpr const char* settingsWindowID = "Settings";
		if (settingsPopup.OpenOnNextFrame)
		{
			Gui::OpenPopup(settingsWindowID);
			settingsPopup.OpenOnNextFrame = false;
			settingsPopup.Window.OnWindowOpen();
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		bool isOpen = true;
		if (Gui::WideBeginPopupModal(settingsWindowID, &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			settingsPopup.WasOpenLastFrame = true;
			settingsPopup.Window.Gui();

			if (settingsPopup.Window.GetAndClearCloseRequestThisFrame())
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}
		else
		{
			settingsPopup.WasOpenLastFrame = false;
		}

		if (!isOpen)
			settingsPopup.Window.OnCloseButtonClicked();
	}

	void ChartEditor::GuiPVScriptImportPopup()
	{
		constexpr const char* pvScriptImportWindowID = "Import PV Script Chart";
		if (pvScriptImportPopup.OpenOnNextFrame)
		{
			Gui::OpenPopup(pvScriptImportWindowID);
			pvScriptImportPopup.OpenOnNextFrame = false;
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		bool isOpen = true;
		if (Gui::WideBeginPopupModal(pvScriptImportWindowID, &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			pvScriptImportPopup.Window.Gui();

			if (pvScriptImportPopup.Window.GetAndClearCloseRequestThisFrame())
			{
				Gui::CloseCurrentPopup();

				if (auto importedChart = pvScriptImportPopup.Window.TryMoveImportedChartBeforeClosing(); importedChart != nullptr)
				{
					chart = std::move(importedChart);
					chart->UpdateMapTimes();

					if (!chart->SongFileName.empty())
						LoadSongAsync(chart->SongFileName);

					SyncWorkingChartPointers();
					undoManager.SetChangesWereMade();
				}
			}

			Gui::EndPopup();
		}

		if (!isOpen)
			pvScriptImportPopup.Window.TryMoveImportedChartBeforeClosing();
	}

	void ChartEditor::GuiPVScriptExportPopup()
	{
		constexpr const char* pvScriptExportWindowID = "Export PV Script MData (Experimental)";
		if (pvScriptExportPopup.OpenOnNextFrame)
		{
			Gui::OpenPopup(pvScriptExportWindowID);
			pvScriptExportPopup.OpenOnNextFrame = false;
			pvScriptExportPopup.Window.OnWindowOpen();
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		bool isOpen = true;
		if (Gui::WideBeginPopupModal(pvScriptExportWindowID, &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			PVExportWindowInputData data = {};
			data.Chart = chart.get();
			data.SoundEffectManager = &soundEffectManager;
			data.SongSampleProvider = Audio::AudioEngine::GetInstance().GetSharedSource(GetSongSource());
			pvScriptExportPopup.Window.Gui(data);

			if (pvScriptExportPopup.Window.GetAndClearCloseRequestThisFrame())
				Gui::CloseCurrentPopup();

			Gui::EndPopup();
		}

		if (!isOpen)
			pvScriptExportPopup.Window.OnCloseButtonClicked();
	}

	void ChartEditor::GuiFileNotFoundPopup()
	{
		constexpr const char* fileNotFoundPopupID = ICON_FA_INFO_CIRCLE "  Comfy Studio - File Not Found##FileNotFoundPopup";

		if (fileNotFoundPopup.OpenOnNextFrame)
		{
			Gui::OpenPopup(fileNotFoundPopupID);
			fileNotFoundPopup.OpenOnNextFrame = false;
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		bool isOpen = true;
		if (Gui::WideBeginPopupModal(fileNotFoundPopupID, &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			constexpr vec2 buttonSize = vec2(120.0f, 0.0f);
			Gui::BeginChild("FileNotFoundPopupText", vec2(0.0f, Gui::GetFontSize() * 4.0f), true, ImGuiWindowFlags_NoBackground);
			Gui::AlignTextToFramePadding();
			Gui::TextWrapped("The file '%.*s' could not be found. %.*s",
				static_cast<i32>(fileNotFoundPopup.NotFoundPath.size()), fileNotFoundPopup.NotFoundPath.c_str(),
				static_cast<i32>(fileNotFoundPopup.QuestionToTheUser.size()), fileNotFoundPopup.QuestionToTheUser.data());
			Gui::EndChild();

			Gui::InvisibleButton("##Dummy", vec2(buttonSize.x * 2.0f, 1.0f));
			Gui::SameLine();

			const bool yesBindingPressed = Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false);
			const bool noBindingPressed = Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_No, false);
			const bool cancelBindingPressed = Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false);

			const bool clickedYes = Gui::Button(ICON_FA_CHECK "   Yes", buttonSize) || yesBindingPressed;
			Gui::SameLine();

			const bool clickedNo = Gui::Button(ICON_FA_TIMES "   No", buttonSize) || noBindingPressed;
			Gui::SameLine();

			if (clickedYes && fileNotFoundPopup.OnYesClickedFunction)
				fileNotFoundPopup.OnYesClickedFunction();

			if (clickedYes || clickedNo || cancelBindingPressed)
			{
				fileNotFoundPopup.NotFoundPath.clear();
				fileNotFoundPopup.QuestionToTheUser = {};
				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}
	}

	void ChartEditor::GuiSaveConfirmationPopup()
	{
		constexpr const char* saveConfirmationPopupID = ICON_FA_INFO_CIRCLE "  Comfy Studio - Unsaved Changes##SaveConfirmationPopup";

		if (saveConfirmationPopup.OpenOnNextFrame)
		{
			Gui::OpenPopup(saveConfirmationPopupID);
			saveConfirmationPopup.OpenOnNextFrame = false;
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		bool isOpen = true;
		if (Gui::WideBeginPopupModal(saveConfirmationPopupID, &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			constexpr vec2 buttonSize = vec2(120.0f, 0.0f);
			Gui::BeginChild("SaveConfirmationPopupText", vec2(0.0f, Gui::GetFontSize() * 3.0f), true, ImGuiWindowFlags_NoBackground);
			Gui::AlignTextToFramePadding();
			Gui::Text("Save changes to the current file?");
			Gui::EndChild();

			const bool yesBindingPressed = Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false);
			const bool noBindingPressed = Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_No, false);
			const bool cancelBindingPressed = Gui::IsWindowFocused() && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false);

			const bool clickedYes = Gui::Button(ICON_FA_CHECK "   Save Changes", buttonSize) || yesBindingPressed;
			Gui::SameLine();

			const bool clickedNo = Gui::Button(ICON_FA_TIMES "   Discard Changes", buttonSize) || noBindingPressed;
			Gui::SameLine();

			const bool clickedCancel = Gui::Button("Cancel", buttonSize) || cancelBindingPressed;

			if (clickedYes || clickedNo || clickedCancel)
			{
				const bool saveDialogCanceled = clickedYes ? !TrySaveNativeChartFileOrOpenDialog() : false;
				UpdateApplicationWindowTitle();

				if (saveConfirmationPopup.OnSuccessFunction)
				{
					if (!clickedCancel && !saveDialogCanceled)
						saveConfirmationPopup.OnSuccessFunction();

					saveConfirmationPopup.OnSuccessFunction = {};
				}

				Gui::CloseCurrentPopup();
			}

			Gui::EndPopup();
		}
	}

	void ChartEditor::SyncWorkingChartPointers()
	{
		if (timeline != nullptr)
			timeline->SetWorkingChart(chart.get());

		if (renderWindow != nullptr)
			renderWindow->SetWorkingChart(chart.get());

		if (playTestWindow != nullptr)
			playTestWindow->SetWorkingChart(chart.get());
	}

	PlayTestWindow& ChartEditor::GetOrCreatePlayTestWindow()
	{
		if (playTestWindow == nullptr)
		{
			PlayTestSharedContext sharedContext = {};
			sharedContext.Renderer = renderer.get();
			sharedContext.RenderHelper = &renderWindow->GetRenderHelper();
			sharedContext.SoundEffectManager = &soundEffectManager;
			sharedContext.ButtonSoundController = buttonSoundController.get();
			sharedContext.SongVoice = &songVoice;
			sharedContext.Chart = chart.get();
			playTestWindow = std::make_unique<PlayTestWindow>(sharedContext);
		}

		return *playTestWindow;
	}

	void ChartEditor::StartPlaytesting(bool startFromCursor)
	{
		PausePlayback();
		playbackTimeOnPlaytestStart = GetPlaybackTimeAsync();
		timelineScrollXOnPlaytestStart = timeline->GetScrollX();

		parentApplication.SetExclusiveFullscreenGui(true);
		const auto cursorTime = std::max(timeline->TickToTime(timeline->TimeToTick(playbackTimeOnPlaytestStart) - BeatTick::FromBars(1)), TimeSpan::Zero());

		auto& playTestWindow = GetOrCreatePlayTestWindow();
		playTestWindow.Restart(startFromCursor ? cursorTime : TimeSpan::Zero());

		if (GlobalUserData.Playtest.EnterFullscreenOnMaximizedStart && parentApplication.GetHost().GetIsMaximized() && !parentApplication.GetHost().GetIsFullscreen())
		{
			parentApplication.GetHost().SetIsFullscreen(true);
			exitFullscreenOnPlaytestEnd = true;
		}
	}

	void ChartEditor::StopPlaytesting(PlayTestExitType exitType)
	{
		assert(exitType != PlayTestExitType::None);
		parentApplication.SetExclusiveFullscreenGui(false);

		PausePlayback();
		if (exitType == PlayTestExitType::ReturnCurrentTime)
		{
			timeline->SetCursorTime(GetPlaybackTimeAsync());
			timeline->CenterCursor();
		}
		else if (exitType == PlayTestExitType::ReturnPrePlayTestTime)
		{
			timeline->SetCursorTime(playbackTimeOnPlaytestStart);
			timeline->SetScrollTargetX(timelineScrollXOnPlaytestStart);
		}

		if (GlobalUserData.Playtest.EnterFullscreenOnMaximizedStart && exitFullscreenOnPlaytestEnd && parentApplication.GetHost().GetIsFullscreen())
			parentApplication.GetHost().SetIsFullscreen(false);
		exitFullscreenOnPlaytestEnd = false;
	}

	bool ChartEditor::GetIsPlayback() const
	{
		return isPlaying;
	}

	void ChartEditor::ResumePlayback()
	{
		playbackTimeOnPlaybackStart = GetPlaybackTimeAsync();

		isPlaying = true;
		songVoice.SetIsPlaying(true);

		timeline->OnPlaybackResumed();
	}

	void ChartEditor::PausePlayback()
	{
		songVoice.SetIsPlaying(false);
		isPlaying = false;

		timeline->OnPlaybackPaused();
	}

	void ChartEditor::StopPlayback()
	{
		SetPlaybackTime(playbackTimeOnPlaybackStart);
		PausePlayback();

		timeline->OnPlaybackStopped();
	}

	Audio::SourceHandle ChartEditor::GetSongSource()
	{
		return songSource;
	}

	Audio::Voice ChartEditor::GetSongVoice()
	{
		return songVoice;
	}
}
