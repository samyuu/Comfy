#include "ChartEditor.h"
#include "ChartCommands.h"
#include "FileFormat/PJEFile.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "IO/Directory.h"
#include "Misc/StringUtil.h"
#include "Core/Application.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	namespace
	{
		static constexpr std::string_view FallbackChartFileName = "Untitled Chart.csfm";
	}

	ChartEditor::ChartEditor(Application& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		chart = std::make_unique<Chart>();
		chart->UpdateMapTimes();

		renderer = std::make_unique<Render::Renderer2D>();

		timeline = std::make_unique<TargetTimeline>(*this, undoManager);
		timeline->SetWorkingChart(chart.get());

		renderWindow = std::make_unique<TargetRenderWindow>(*this, *timeline, undoManager, *renderer);
		renderWindow->SetWorkingChart(chart.get());

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

		UpdateApplicationWindowTitle();
		UpdateAsyncSongSourceLoading();

		GuiSubWindows();
		GuiSaveConfirmationPopup();

		undoManager.FlushExecuteEndOfFrameCommands();
	}

	void ChartEditor::GuiMenu()
	{
		// TODO: Restructure all of this, should probably be part of the currently active editor component (?); Dummy menus for now
		if (Gui::BeginMenu("File"))
		{
			if (Gui::MenuItem("New Chart", nullptr, false, true))
				CheckOpenSaveConfirmationPopupThenCall([this] { CreateNewChart(); });

			if (Gui::MenuItem("Open...", nullptr, false, true))
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

			if (Gui::MenuItem("Export...", nullptr, false, false))
			{
				// TODO:
			}

			Gui::Separator();

			if (Gui::MenuItem("Exit", "Alt + F4"))
				CheckOpenSaveConfirmationPopupThenCall([this] { parentApplication.Exit(); });

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

	bool ChartEditor::IsAudioFile(std::string_view filePath)
	{
		return IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(filePath), ".flac;.ogg;.mp3;.wav");
	}

	bool ChartEditor::OpenLoadAudioFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
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

	bool ChartEditor::OnFileDropped(const std::string& filePath)
	{
		if (!IsAudioFile(filePath))
			return false;

		LoadSongAsync(filePath);
		return true;
	}

	void ChartEditor::LoadSongAsync(std::string_view filePath)
	{
		UnloadSong();

		// TODO: Song file name relative to chart directory
		songSourceFuture = Audio::AudioEngine::GetInstance().LoadAudioSourceAsync(filePath);
		songSourceFilePath = std::string(filePath);

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

		const auto chartFile = IO::File::Load<ComfyStudioChartFile>(filePath);
		if (chartFile == nullptr)
			return;

		undoManager.ClearAll();

		// TODO: Additional processing, setting up file paths etc.
		chart = chartFile->ToChart();
		chart->UpdateMapTimes();
		chart->ChartFilePath = std::string(filePath);

		const auto chartDirectory = IO::Path::GetDirectoryName(filePath);
		const auto songFilePath = !chart->SongFileName.empty() ? IO::Path::Combine(chartDirectory, chart->SongFileName) : "";

		LoadSongAsync(songFilePath);

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
		}
	}

	bool ChartEditor::OpenReadChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
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
		fileDialog.FileName =
			!chart->ChartFilePath.empty() ? IO::Path::GetFileName(chart->ChartFilePath) :
			!chart->Properties.Song.Title.empty() ? chart->Properties.Song.Title :
			FallbackChartFileName;

		fileDialog.DefaultExtension = ComfyStudioChartFile::Extension;
		fileDialog.Filters = { { std::string(ComfyStudioChartFile::FilterName), std::string(ComfyStudioChartFile::FilterSpec) }, };
		// TODO: Option to copy audio file into output directory if absolute path
		// fileDialog.CustomizeItems;
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return false;

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
		chart = (pjeFile != nullptr) ? pjeFile->ToChart() : std::make_unique<Chart>();
		chart->UpdateMapTimes();
		LoadSongAsync((pjeFile != nullptr) ? pjeFile->TryFindSongFilePath(filePath) : "");

		timeline->SetWorkingChart(chart.get());
		renderWindow->SetWorkingChart(chart.get());
	}

	bool ChartEditor::OpenReadImportChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		fileDialog.DefaultExtension = Legacy::PJEFile::Extension;
		fileDialog.Filters = { { std::string(Legacy::PJEFile::FilterName), std::string(Legacy::PJEFile::FilterSpec) }, };
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenRead())
			return false;

		ImportChartFileSync(fileDialog.OutFilePath);
		return true;
	}

	bool ChartEditor::IsChartStateSyncedToFile() const
	{
		// TODO: Compare data against lastSavedChartFile (?)
		//		 or set "isDirty" flag when an action is executed (wouldn't work with non undoable song info edits)
		return undoManager.GetUndoStackView().empty();
	}

	void ChartEditor::CheckOpenSaveConfirmationPopupThenCall(std::function<void()> onSuccess)
	{
		if (!IsChartStateSyncedToFile())
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

	void ChartEditor::UpdateApplicationWindowTitle()
	{
		const auto fileName = chart->ChartFilePath.empty() ? FallbackChartFileName : IO::Path::GetFileName(chart->ChartFilePath, true);

		// NOTE: Comparing a few characters each frame should be well worth not having to manually sync the window title every time the chart file path is updated externally
		if (fileName != lastSetWindowTitle)
		{
			parentApplication.SetFormattedWindowTitle(fileName);
			lastSetWindowTitle = fileName;
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
		}

		if (chart->Duration <= TimeSpan::Zero())
			chart->Duration = songVoice.GetDuration();

		auto& songInfo = chart->Properties.Song;
		{
			// TODO: Extract metadata from audio file
			if (songInfo.Title.empty())
				songInfo.Title = IO::Path::GetFileName(songSourceFilePath, false);

			songInfo.Artist;
			songInfo.Album;
		}

		SetPlaybackTime(oldPlaybackTime);
		timeline->OnSongLoaded();
	}

	void ChartEditor::GuiSubWindows()
	{
#if 1 // TODO: Remove in favor of the chart properties window song file name input widget
		if (Gui::Begin(ICON_FA_FOLDER "  Song Loader##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			Gui::BeginChild("SongLoaderChild##ChartEditor");

			songFileViewer.SetIsReadOnly(IsSongAsyncLoading());
			if (songFileViewer.DrawGui())
			{
				if (IsAudioFile(songFileViewer.GetFileToOpen()))
					LoadSongAsync(songFileViewer.GetFileToOpen());
			}
			Gui::EndChild();
		}
		Gui::End();
#endif

		if (Gui::Begin(ICON_FA_MUSIC "  Target Timeline##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			timeline->DrawTimelineGui();
		}
		Gui::End();

		if (Gui::Begin(ICON_FA_SYNC "  Sync Window##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			syncWindow.Gui(*chart, *timeline);
		}
		Gui::End();

		if (Gui::Begin(ICON_FA_INFO_CIRCLE "  Target Inspector##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			inspector.Gui(*chart);
		}
		Gui::End();

		renderWindow->BeginEndGui(ICON_FA_CHART_BAR "  Target Preview##ChartEditor");

		if (Gui::Begin(ICON_FA_HISTORY "  Chart Editor History##ChartEditor"))
		{
			historyWindow.Gui();
		}
		Gui::End();

		if (Gui::Begin(ICON_FA_STOPWATCH "  BPM Calculator##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			bpmCalculatorWindow.Gui(*chart, GetIsPlayback() ? GetPlaybackTimeOnPlaybackStart() : GetPlaybackTimeAsync());
		}
		Gui::End();

		if (Gui::Begin(ICON_FA_LIST_UL "  Chart Properties", nullptr, ImGuiWindowFlags_None))
		{
			chartPropertiesWindow.Gui(*chart);
		}
		Gui::End();
	}

	void ChartEditor::GuiSaveConfirmationPopup()
	{
		if (saveConfirmationPopup.OpenOnNextFrame)
		{
			Gui::OpenPopup(saveConfirmationPopup.ID);
			saveConfirmationPopup.OpenOnNextFrame = false;
		}

		const auto* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos + (viewport->Size / 2.0f), ImGuiCond_Appearing, vec2(0.5f));

		if (Gui::BeginPopupModal(saveConfirmationPopup.ID, nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Gui::Text("Save changes to the current file?\n\n\n");
			Gui::Separator();

			const bool clickedYes = Gui::Button("Yes", vec2(120.0f, 0.0f));
			Gui::SameLine();
			const bool clickedNo = Gui::Button("No", vec2(120.0f, 0.0f));
			Gui::SameLine();
			const bool clickedCancel = Gui::Button("Cancel", vec2(120.0f, 0.0f)) || (Gui::IsWindowFocused() && Gui::IsKeyPressed(Input::KeyCode_Escape));

			if (clickedYes || clickedNo || clickedCancel)
			{
				const bool saveDialogCanceled = clickedYes ? !TrySaveChartFileOrOpenDialog() : false;
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
