#include "ChartEditor.h"
#include "ChartCommands.h"
#include "FileFormat/PJEFile.h"
#include "IO/Path.h"
#include "IO/Shell.h"
#include "Misc/StringUtil.h"
#include "Core/Application.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
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
		UpdateAsyncSongSourceLoading();

		if (Gui::Begin(ICON_FA_FOLDER "  Song Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			Gui::BeginChild("SongLoaderChild##ChartEditor");

			songFileViewer.SetIsReadOnly(IsSongAsyncLoading());
			if (songFileViewer.DrawGui())
			{
				if (IsAudioFile(songFileViewer.GetFileToOpen()))
					LoadSongAsync(songFileViewer.GetFileToOpen());
				else // TEMP: For quick testing, chart files should be loaded through a menu item instead
					LoadChartFileSync(songFileViewer.GetFileToOpen());
			}
			Gui::EndChild();
		}
		Gui::End();

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

		undoManager.FlushExecuteEndOfFrameCommands();
	}

	bool ChartEditor::IsAudioFile(std::string_view filePath)
	{
		return IO::Path::DoesAnyPackedExtensionMatch(IO::Path::GetExtension(filePath), ".flac;.ogg;.mp3;.wav");
	}

	void ChartEditor::OpenLoadAudioFileDialog()
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

		if (fileDialog.OpenRead())
			LoadSongAsync(IO::Path::Normalize(fileDialog.OutFilePath));
	}

	bool ChartEditor::OnFileDropped(const std::string& filePath)
	{
		return (IsAudioFile(filePath) && LoadSongAsync(filePath));
	}

	bool ChartEditor::LoadSongAsync(std::string_view filePath)
	{
		UnloadSong();

		songSourceFuture = Audio::AudioEngine::GetInstance().LoadAudioSourceAsync(filePath);
		songSourceFilePath = std::string(filePath);

		// NOTE: Clear file name here so the chart properties window help loading text is displayed
		//		 then set again once the song audio file has finished loading
		chart->SongFileName.clear();

		return true;
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

	void ChartEditor::LoadChartFileSync(std::string_view filePath)
	{
		// DEBUG: Only support pje for now
		if (!Util::EndsWithInsensitive(songFileViewer.GetFileToOpen(), Legacy::PJEFile::Extension))
			return;

		const auto pjeFile = IO::File::Load<Legacy::PJEFile>(filePath);
		if (pjeFile == nullptr)
			return;

		undoManager.ClearAll();

		// TODO: Additional processing, setting up file paths etc.
		chart = pjeFile->ToChart();
		chart->UpdateMapTimes();
		chart->ChartFilePath = std::string(filePath);

		auto getChartSongFilePath = [&]() -> std::string
		{
			const auto chartDirectory = IO::Path::GetDirectoryName(filePath);
			if (!chart->SongFileName.empty())
				return IO::Path::Combine(chartDirectory, chart->SongFileName);

			const auto filePathNoExtension = IO::Path::Combine(chartDirectory, IO::Path::GetFileName(chart->Properties.Song.Title, false));
			std::string combinedPath;
			combinedPath.reserve(filePathNoExtension.size() + 5);

			for (const auto extension : std::array { ".flac", ".ogg", ".mp3", ".wav" })
			{
				combinedPath.clear();
				combinedPath += filePathNoExtension;
				combinedPath += extension;

				if (IO::File::Exists(combinedPath))
					return combinedPath;
			}

			return "";
		};

		LoadSongAsync(getChartSongFilePath());

		timeline->SetWorkingChart(chart.get());
		renderWindow->SetWorkingChart(chart.get());
	}

	void ChartEditor::SaveChartFileAsync(std::string_view filePath)
	{
		if (!filePath.empty())
			chart->ChartFilePath = filePath;

		if (!chart->ChartFilePath.empty())
		{
			// TODO: Implement
			assert(false);

			/*
			[this] std::future<bool> chartSaveFuture;
			[this] std::unique_ptr<ComfyStudioChartFile> chartFile;

			if (chartSaveFuture.valid())
				chartSaveFuture.get();

			chartFile = std::make_unique<ComfyStudioChartFile>(*chart);
			chartSaveFuture = IO::File::SaveAsync(chart->ChartFilePath, *chartFile);
			*/
		}
		else
		{
			// TODO: No path specified... maybe write to some temp directory instead (?)
			assert(false);
		}
	}

	void ChartEditor::OpenSaveChartFileDialog()
	{
		IO::Shell::FileDialog fileDialog;
		// TODO: Default name based on song title
		fileDialog.FileName = chart->ChartFilePath;
		fileDialog.DefaultExtension = ""; // ComfyStudioChartFile::Extension;
		fileDialog.Filters = { { "Comfy Studio Chart File (*.csfm)", "*.csfm" }, };
		// TODO: Option to copy audio file into output directory if absolute path
		// fileDialog.CustomizeItems;
		fileDialog.ParentWindowHandle = Application::GetGlobalWindowFocusHandle();

		if (!fileDialog.OpenSave())
			return;

		SaveChartFileAsync(fileDialog.OutFilePath);
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

		if (chart->Duration <= TimeSpan::Zero() || chart->ChartFilePath.empty())
			chart->Duration = songVoice.GetDuration();

		if (chart->ChartFilePath.empty())
		{
			// TODO: Extract metadata from audio file
			chart->Properties.Song.Title = IO::Path::GetFileName(songSourceFilePath, false);
			chart->Properties.Song.Artist;
			chart->Properties.Song.Album;
		}

		timeline->OnSongLoaded();
		SetPlaybackTime(oldPlaybackTime);
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
