#include "ChartEditor.h"
#include "ChartCommands.h"
#include "KeyBindings.h"
#include "FileFormat/PJEFile.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "IO/Directory.h"
#include "Core/Application.h"
#include "Misc/StringUtil.h"
#include "System/ComfyData.h"
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

	ChartEditor::ChartEditor(Application& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		chart = std::make_unique<Chart>();
		chart->UpdateMapTimes();

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
			LoadChartFileSync(fileToOpen);
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

		UpdateApplicationClosingRequest();
		UpdateGlobalControlInput();
		UpdateApplicationWindowTitle();
		UpdateAsyncSongSourceLoading();

		GuiSubWindows();
		GuiSaveConfirmationPopup();

		undoManager.FlushExecuteEndOfFrameCommands();
	}

	void ChartEditor::GuiMenu()
	{
		if (Gui::BeginMenu("File"))
		{
			if (Gui::MenuItem("New Chart", nullptr, false, true))
				CheckOpenSaveConfirmationPopupThenCall([this] { CreateNewChart(); });

			if (Gui::MenuItem("Open...", "Ctrl + O", false, true))
				CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadChartFileDialog(); });

			if (Gui::BeginMenu("Open Recent", false))
			{
				// TODO: Read and write to / from config file (?)
				for (const auto path : std::array { "dev_ram/chart/test/test_chart.csfm" })
				{
					if (Gui::MenuItem(path))
						CheckOpenSaveConfirmationPopupThenCall([this, path] { LoadChartFileSync(path); });
				}

				// TODO: Clear config file entries
				if (Gui::MenuItem("Clear Items")) {}

				Gui::EndMenu();
			}

			const auto chartDirectory = IO::Path::GetDirectoryName(chart->ChartFilePath);
			if (Gui::MenuItem("Open Chart Directory...", nullptr, false, !chartDirectory.empty()))
			{
				if (IO::Directory::Exists(chartDirectory))
					IO::Shell::OpenInExplorer(chartDirectory);
			}

			Gui::Separator();

			if (Gui::MenuItem("Save", "Ctrl + S", false, true))
				TrySaveChartFileOrOpenDialog();
			if (Gui::MenuItem("Save As...", "Ctrl + Shift + S", false, true))
				OpenSaveChartFileDialog();
			Gui::Separator();

			if (Gui::MenuItem("Import...", nullptr, false, true))
				CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadImportChartFileDialog(); });

			if (Gui::MenuItem("Export As...", nullptr, false, true))
				OpenSaveExportChartFileDialog();

			Gui::Separator();

			if (Gui::MenuItem("Exit", "Alt + F4"))
				applicationExitRequested = true;

			Gui::EndMenu();
		}

		if (Gui::BeginMenu("Edit"))
		{
			if (Gui::MenuItem("Undo", "Ctrl + Z", false, undoManager.CanUndo()))
				undoManager.Undo();
			if (Gui::MenuItem("Redo", "Ctrl + Y", false, undoManager.CanRedo()))
				undoManager.Redo();
			Gui::Separator();

			// TODO: Or maybe this should be part of the EditorManager instead so that each component can register its own sub settings menu (?)
			if (Gui::MenuItem("Settings...", nullptr, false, false)) {}

			Gui::EndMenu();
		}

#if COMFY_DEBUG || 1 // TODO:
		static bool debugPopupPlaytestWindow = false;
		if (debugPopupPlaytestWindow)
		{
			if (Gui::Begin("Popout Playtest Window (Debug)", &debugPopupPlaytestWindow, (ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoDocking)))
				OnExclusiveGui();
			Gui::End();
		}

		if (Gui::BeginMenu("Playtest"))
		{
			if (Gui::MenuItem("Start from Beginning", Input::GetKeyCodeName(KeyBindings::StartPlaytestFromStart)))
				StartPlaytesting(false);

			if (Gui::MenuItem("Start from Cursor", Input::GetKeyCodeName(KeyBindings::StartPlaytestFromCursor)))
				StartPlaytesting(true);

			Gui::Separator();
			bool autoplayEnabled = (playTestWindow == nullptr) ? false : playTestWindow->GetAutoplayEnabled();
			if (Gui::MenuItem("Autoplay Enabled", nullptr, &autoplayEnabled))
				GetOrCreatePlayTestWindow().SetAutoplayEnabled(autoplayEnabled);

#if COMFY_DEBUG && 0
			if (Gui::MenuItem("Popout Playtest Window (Debug)", nullptr, &debugPopupPlaytestWindow)) {}
#endif

			Gui::EndMenu();
		}
#endif
	}

	void ChartEditor::OnEditorComponentMadeActive()
	{
		lastSetWindowTitle.clear();
	}

	ApplicationHostCloseResponse ChartEditor::OnApplicationClosing()
	{
		if (undoManager.GetHasPendingChanged())
		{
			if (parentApplication.GetExclusiveFullscreenGui())
				StopPlaytesting();

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
		auto& playTestWindow = GetOrCreatePlayTestWindow();
		playTestWindow.ExclusiveGui();

		if (playTestWindow.ExitRequestedThisFrame())
			StopPlaytesting();
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
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		LoadSongAsync(IO::Path::Normalize(fileDialog.OutFilePath));
		return true;
	}

	bool ChartEditor::OnFileDropped(std::string_view filePath)
	{
		if (Util::EndsWithInsensitive(filePath, ComfyStudioChartFile::Extension))
		{
			CheckOpenSaveConfirmationPopupThenCall([this, path = std::string(filePath)] { LoadChartFileSync(path); });
			return true;
		}

		if (IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(filePath), ".flac;.ogg;.mp3;.wav"))
		{
			LoadSongAsync(filePath);
			return true;
		}

		if (IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(filePath), ".png;.jpg;.jpeg;.bmp;.gif"))
		{
			auto setAndLoad = [&](std::string& outFileName, AsyncLoadedImageFile& image)
			{
				image.TryLoad(filePath);
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

		if (isPlaying)
			PausePlayback();

		timeline->OnSongLoaded();
	}

	void ChartEditor::CreateNewChart()
	{
		undoManager.ClearAll();
		chart = std::make_unique<Chart>();
		chart->UpdateMapTimes();
		UnloadSong();

		SyncWorkingChartPointers();
	}

	void ChartEditor::LoadChartFileSync(std::string_view filePath)
	{
		if (!Util::EndsWithInsensitive(filePath, ComfyStudioChartFile::Extension))
			return;

		// TODO: Display error GUI if unable to load
		const auto chartFile = IO::File::Load<ComfyStudioChartFile>(filePath);

		undoManager.ClearAll();

		// TODO: Additional processing, setting up file paths etc.
		chart = (chartFile != nullptr) ? chartFile->MoveToChart() : std::make_unique<Chart>();
		chart->UpdateMapTimes();
		chart->ChartFilePath = std::string(filePath);

		if (!chart->SongFileName.empty())
			LoadSongAsync(chart->SongFileName);
		else
			UnloadSong();

		chart->Properties.Image.Cover.TryLoad(chart->Properties.Image.CoverFileName, chart->ChartFilePath);
		chart->Properties.Image.Logo.TryLoad(chart->Properties.Image.LogoFileName, chart->ChartFilePath);
		chart->Properties.Image.Background.TryLoad(chart->Properties.Image.BackgroundFileName, chart->ChartFilePath);

		SyncWorkingChartPointers();
	}

	void ChartEditor::SaveChartFileAsync(std::string_view filePath)
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
		}
	}

	bool ChartEditor::OpenReadChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Open Chart";
		fileDialog.DefaultExtension = ComfyStudioChartFile::Extension;
		fileDialog.Filters = { { std::string(ComfyStudioChartFile::FilterName), std::string(ComfyStudioChartFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		LoadChartFileSync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::OpenSaveChartFileDialog()
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

		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

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

		SaveChartFileAsync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::TrySaveChartFileOrOpenDialog()
	{
		if (chart->ChartFilePath.empty())
			return OpenSaveChartFileDialog();
		else
			SaveChartFileAsync();
		return true;
	}

	void ChartEditor::ImportChartFileSync(std::string_view filePath)
	{
		// DEBUG: Only support pje for now
		const auto pjeFile = Util::EndsWithInsensitive(filePath, Legacy::PJEFile::Extension) ? IO::File::Load<Legacy::PJEFile>(filePath) : nullptr;

		undoManager.ClearAll();
		undoManager.SetChangesWereMade();

		chart = (pjeFile != nullptr) ? pjeFile->ToChart() : std::make_unique<Chart>();
		chart->UpdateMapTimes();

		// NOTE: Unable to use relative file path because imported chart data don't and shouldn't set a working directory
		LoadSongAsync((pjeFile != nullptr) ? pjeFile->TryFindSongFilePath(filePath) : "");

		SyncWorkingChartPointers();
	}

	void ChartEditor::ExportChartFileSync(std::string_view filePath)
	{
		// TODO: Implement different export options
		if (Util::EndsWithInsensitive(filePath, Legacy::PJEFile::Extension))
		{
			auto pjeFile = Legacy::PJEFile(*chart);
			IO::File::Save(filePath, pjeFile);
		}
	}

	bool ChartEditor::OpenReadImportChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Import Chart";
		fileDialog.DefaultExtension = Legacy::PJEFile::Extension;
		fileDialog.Filters = { { std::string(Legacy::PJEFile::FilterName), std::string(Legacy::PJEFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		ImportChartFileSync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::OpenSaveExportChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Export Chart As";
		fileDialog.FileName = GetChartSaveDialogFileName(*chart);
		fileDialog.DefaultExtension = Legacy::PJEFile::Extension;
		fileDialog.Filters = { { std::string(Legacy::PJEFile::FilterName), std::string(Legacy::PJEFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return false;

		ExportChartFileSync(fileDialog.OutFilePath);
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
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

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
		return songVoice.GetPosition() - chart->StartOffset;
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

		const bool shift = Gui::GetIO().KeyShift;

		if (Gui::IsKeyPressed(KeyBindings::StartPlaytestFromStart, false))
			StartPlaytesting(false);

		if (Gui::IsKeyPressed(KeyBindings::StartPlaytestFromCursor, false))
			StartPlaytesting(true);

		if (Gui::GetIO().KeyCtrl)
		{
			if (Gui::IsKeyPressed(KeyBindings::Undo, true))
				undoManager.Undo();

			if (Gui::IsKeyPressed(KeyBindings::Redo, true))
				undoManager.Redo();

			if (Gui::IsKeyPressed(Input::KeyCode_O, false) && !shift)
				CheckOpenSaveConfirmationPopupThenCall([this] { OpenReadChartFileDialog(); });

			if (Gui::IsKeyPressed(Input::KeyCode_S, false) && !shift)
				TrySaveChartFileOrOpenDialog();

			if (Gui::IsKeyPressed(Input::KeyCode_S, false) && shift)
				OpenSaveChartFileDialog();
		}
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

	void ChartEditor::GuiSubWindows()
	{
		if (Gui::Begin(ICON_FA_DRAFTING_COMPASS "  Sync Target Presets", nullptr, ImGuiWindowFlags_None))
			presetWindow.SyncGui(*chart);
		Gui::End();

#if 0 // TODO:
		if (Gui::Begin(ICON_FA_DRAFTING_COMPASS "  Sequence Target Presets", nullptr, ImGuiWindowFlags_None))
			presetWindow.SequenceGui(*chart);
		Gui::End();
#endif

		if (Gui::Begin(ICON_FA_MUSIC "  Target Timeline##ChartEditor", nullptr, ImGuiWindowFlags_None))
			timeline->DrawTimelineGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_STOPWATCH "  BPM Calculator##ChartEditor", nullptr, ImGuiWindowFlags_None))
			bpmCalculatorWindow.Gui(*chart, GetIsPlayback() ? GetPlaybackTimeOnPlaybackStart() : GetPlaybackTimeAsync());
		Gui::End();

		if (Gui::Begin(ICON_FA_SYNC "  Sync Window##ChartEditor", nullptr, ImGuiWindowFlags_None))
			syncWindow.Gui(*chart, *timeline);
		Gui::End();

		if (Gui::Begin(ICON_FA_INFO_CIRCLE "  Target Inspector##ChartEditor", nullptr, ImGuiWindowFlags_None))
			inspector.Gui(*chart);
		Gui::End();

		renderWindow->BeginEndGui(ICON_FA_CHART_BAR "  Target Preview##ChartEditor");

		if (Gui::Begin(ICON_FA_HISTORY "  Chart Editor History##ChartEditor"))
			historyWindow.Gui();
		Gui::End();

		if (Gui::Begin(ICON_FA_LIST_UL "  Chart Properties", nullptr, ImGuiWindowFlags_None))
			chartPropertiesWindow.Gui(*chart);
		Gui::End();
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
		if (Gui::BeginPopupModal(saveConfirmationPopupID, &isOpen, ImGuiWindowFlags_AlwaysAutoResize))
		{
			constexpr auto buttonSize = vec2(120.0f, 0.0f);

			Gui::Text("  Save changes to the current file?\n\n\n");
			Gui::Separator();

			const bool clickedYes = Gui::Button(ICON_FA_CHECK "   Save Changes", buttonSize) || (Gui::IsWindowFocused() && Gui::IsKeyPressed(Input::KeyCode_Enter, false));
			Gui::SameLine();

			const bool clickedNo = Gui::Button(ICON_FA_TIMES "   Discard Changes", buttonSize);
			Gui::SameLine();

			const bool clickedCancel = Gui::Button("Cancel", buttonSize) || (Gui::IsWindowFocused() && Gui::IsKeyPressed(Input::KeyCode_Escape, false));

			if (clickedYes || clickedNo || clickedCancel)
			{
				const bool saveDialogCanceled = clickedYes ? !TrySaveChartFileOrOpenDialog() : false;
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

		parentApplication.SetExclusiveFullscreenGui(true);
		const auto cursorTime = std::max(timeline->TickToTime(timeline->TimeToTick(playbackTimeOnPlaytestStart) - BeatTick::FromBars(1)), TimeSpan::Zero());

		auto& playTestWindow = GetOrCreatePlayTestWindow();
		playTestWindow.Restart(startFromCursor ? cursorTime : TimeSpan::Zero());
	}

	void ChartEditor::StopPlaytesting()
	{
		parentApplication.SetExclusiveFullscreenGui(false);

		// TODO: Option to continue at playtest replay point (?)
		PausePlayback();
		timeline->SetCursorTime(playbackTimeOnPlaytestStart);
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
