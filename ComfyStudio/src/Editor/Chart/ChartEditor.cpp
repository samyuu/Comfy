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

		renderer = std::make_unique<Render::Renderer2D>();

		// TODO: Load async (?)
		editorSprites = System::Data.Load<Graphics::SprSet>(System::Data.FindFile("sprite/spr_chart_editor.bin"));

		timeline = std::make_unique<TargetTimeline>(*this, undoManager);
		timeline->SetWorkingChart(chart.get());
		timeline->OnEditorSpritesLoaded(editorSprites.get());

		renderWindow = std::make_unique<TargetRenderWindow>(*this, *timeline, undoManager, *renderer);
		renderWindow->SetWorkingChart(chart.get());

		presetWindow.OnEditorSpritesLoaded(editorSprites.get());

		songVoice = Audio::AudioEngine::GetInstance().AddVoice(Audio::SourceHandle::Invalid, "ChartEditor::SongVoice", false, 0.75f, true);

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
	}

	void ChartEditor::OnEditorComponentMadeActive()
	{
		lastSetWindowTitle.clear();
	}

	ApplicationHostCloseResponse ChartEditor::OnApplicationClosing()
	{
		if (undoManager.GetHasPendingChanged())
		{
			applicationExitRequested = true;
			return ApplicationHostCloseResponse::SupressExit;
		}
		else
		{
			return ApplicationHostCloseResponse::Exit;
		}
	}

	bool ChartEditor::IsAudioFile(std::string_view filePath)
	{
		return IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(filePath), ".flac;.ogg;.mp3;.wav");
	}

	bool ChartEditor::OpenLoadAudioFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Open";
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

		if (IsAudioFile(filePath))
		{
			LoadSongAsync(filePath);
			return true;
		}

		return false;
	}

	void ChartEditor::LoadSongAsync(std::string_view filePath)
	{
		UnloadSong();

		if (!chart->ChartFilePath.empty() && IO::Path::IsRelative(filePath))
			songSourceFilePath = IO::Path::Combine(IO::Path::GetDirectoryName(chart->ChartFilePath), chart->SongFileName);
		else
			songSourceFilePath = filePath;

		songSourceFuture = Audio::AudioEngine::GetInstance().LoadAudioSourceAsync(songSourceFilePath);

		// NOTE: Clear file name here so the chart properties window help loading text is displayed
		//		 then set again once the song audio file has finished loading
		chart->SongFileName.clear();
	}

	void ChartEditor::UnloadSong()
	{
		// NOTE: Optimally the last action should be canceled through a stop token instead
		if (songSourceFuture.valid())
			Audio::AudioEngine::GetInstance().UnloadSource(songSourceFuture.get());

		Audio::AudioEngine::GetInstance().UnloadSource(songSource);
		songSource = Audio::SourceHandle::Invalid;
		songSourceFilePath.clear();

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

		timeline->SetWorkingChart(chart.get());
		renderWindow->SetWorkingChart(chart.get());
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

		timeline->SetWorkingChart(chart.get());
		renderWindow->SetWorkingChart(chart.get());
	}

	void ChartEditor::SaveChartFileAsync(std::string_view filePath)
	{
		if (!filePath.empty())
			chart->ChartFilePath = filePath;

		if (!chart->ChartFilePath.empty())
		{
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
		fileDialog.Title = "Open";
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
		fileDialog.Title = "Save As";
		fileDialog.FileName = GetChartSaveDialogFileName(*chart);
		fileDialog.DefaultExtension = ComfyStudioChartFile::Extension;
		fileDialog.Filters = { { std::string(ComfyStudioChartFile::FilterName), std::string(ComfyStudioChartFile::FilterSpec) }, };

		const bool songFileIsAbsolute = !chart->SongFileName.empty() && !IO::Path::IsRelative(chart->SongFileName);
		const bool copySongFileDefaultValue = false;

		bool copySongFile = copySongFileDefaultValue;
		fileDialog.CustomizeItems = { { IO::Shell::Custom::ItemType::Checkbox, "Copy Song to Directory", songFileIsAbsolute ? &copySongFile : nullptr } };

		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return false;

		if (copySongFile && songFileIsAbsolute)
		{
			const auto songFileName = IO::Path::GetFileName(chart->SongFileName);
			const auto chartDirectory = IO::Path::GetDirectoryName(fileDialog.OutFilePath);
			const auto chartDirectorySongPath = IO::Path::Combine(chartDirectory, songFileName);

			// TODO: Consider doing this async although since this only happens *once* in the save *dialog* it shouldn't be too noticable
			if (IO::File::Copy(chart->SongFileName, chartDirectorySongPath))
			{
				chart->SongFileName = songFileName;
				songSourceFilePath = chartDirectorySongPath;
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

		timeline->SetWorkingChart(chart.get());
		renderWindow->SetWorkingChart(chart.get());
	}

	void ChartEditor::ExportChartFileSync(std::string_view filePath)
	{
		// TODO: Implement different export options
		if (Util::EndsWithInsensitive(filePath, Legacy::PJEFile::Extension))
		{
			// TODO: Implement IStreamWritable interface for PJEFile and FromChart constructor
			auto pjeFile = Legacy::PJEFile(*chart);
			IO::File::Save(filePath, pjeFile);
		}
	}

	bool ChartEditor::OpenReadImportChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.Title = "Import";
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
		fileDialog.Title = "Export As";
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
		// TODO: Undo / redo controls for all child windows
		// TODO: Move all keycodes into KeyBindings header and implement modifier + keycode string format helper

		// HACK: Works for now I guess...
		if (!parentApplication.GetHost().IsWindowFocused())
			return;

		if (Gui::GetIO().KeyCtrl && Gui::GetActiveID() == 0)
		{
			const bool shift = Gui::GetIO().KeyShift;

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

		if (chart->SongFileName.empty())
		{
			const auto chartDirectory = IO::Path::Normalize(IO::Path::GetDirectoryName(chart->ChartFilePath));
			const auto songDirectory = IO::Path::Normalize(IO::Path::GetDirectoryName(songSourceFilePath));

			chart->SongFileName = (!chartDirectory.empty() && chartDirectory == songDirectory) ? IO::Path::GetFileName(songSourceFilePath, true) : songSourceFilePath;

			// NOTE: Should already set by the properties window on user interaction
			// undoManager.SetChangesWereMade();
		}

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
				songInfo.Title = IO::Path::GetFileName(songSourceFilePath, false);
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
#if 0 // TODO:
		if (Gui::Begin(ICON_FA_QUESTION_CIRCLE "  Chart Editor Manual", nullptr, ImGuiWindowFlags_None))
			chartEditorManual.Gui();
		Gui::End();
#endif

		if (Gui::Begin(ICON_FA_DRAFTING_COMPASS "  Sync Target Presets", nullptr, ImGuiWindowFlags_None))
			presetWindow.SyncGui(*chart);
		Gui::End();

		if (Gui::Begin(ICON_FA_DRAFTING_COMPASS "  Sequence Target Presets", nullptr, ImGuiWindowFlags_None))
			presetWindow.SequenceGui(*chart);
		Gui::End();

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
