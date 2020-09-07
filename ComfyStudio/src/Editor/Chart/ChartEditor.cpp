#include "ChartEditor.h"
#include "Misc/StringUtil.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	ChartEditor::ChartEditor(Application& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		chart = std::make_unique<Chart>();
		renderer = std::make_unique<Render::Renderer2D>();

		timeline = std::make_unique<TargetTimeline>(*this, undoManager);
		renderWindow = std::make_unique<TargetRenderWindow>(*this, *timeline, undoManager, *renderer);


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

			songFileViewer.SetIsReadOnly(songSourceFuture.valid() && !songSourceFuture._Is_ready());
			if (songFileViewer.DrawGui())
			{
				if (IsAudioFile(songFileViewer.GetFileToOpen()))
					LoadSongAsync(songFileViewer.GetFileToOpen());
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

		// TODO:
		// renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
		// renderWindow->SetIsPlayback(timeline->GetIsPlayback());
		// renderWindow->SetCurrentFrame(timeline->GetFrame().Frames());
		renderWindow->BeginEndGui(ICON_FA_CHART_BAR "  Target Window##ChartEditor");

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

		undoManager.FlushExecuteEndOfFrameCommands();
	}

	bool ChartEditor::IsAudioFile(std::string_view filePath)
	{
		for (const auto& fileExtension : audioFileExtensions)
		{
			if (Util::EndsWithInsensitive(filePath, fileExtension))
				return true;
		}

		return false;
	}

	bool ChartEditor::OnFileDropped(const std::string& filePath)
	{
		return (IsAudioFile(filePath) && LoadSongAsync(filePath));
	}

	bool ChartEditor::LoadSongAsync(std::string_view filePath)
	{
		songSourceFuture = Audio::AudioEngine::GetInstance().LoadAudioSourceAsync(filePath);
		songSourceFilePath = std::string(filePath);

		return true;
	}

	TimeSpan ChartEditor::GetPlaybackTimeAsync() const
	{
		return songVoice.GetPosition() - chart->StartOffset;
	}

	void ChartEditor::SetPlaybackTime(TimeSpan value)
	{
		songVoice.SetPosition(value + chart->StartOffset);
	}

	Chart* ChartEditor::GetChart()
	{
		return chart.get();
	}

	TimeSpan ChartEditor::GetPlaybackTimeOnPlaybackStart() const
	{
		return playbackTimeOnPlaybackStart;
	}

	void ChartEditor::UpdateAsyncSongSourceLoading()
	{
		if (!songSourceFuture.valid() || !songSourceFuture._Is_ready())
			return;

		const auto previousPlaybackTime = GetPlaybackTimeAsync();
		const auto newSongStream = songSourceFuture.get();

		Audio::AudioEngine::GetInstance().UnloadSource(songSource);

		songVoice.SetSource(newSongStream);
		songSource = newSongStream;

		undoManager.Execute<ChangeSongDuration>(*chart, (songVoice.GetDuration() <= TimeSpan::Zero()) ? Chart::FallbackDuration : songVoice.GetDuration());
		undoManager.Execute<ChangeSongName>(*chart, std::string(IO::Path::GetFileName(songSourceFilePath, false)));

		timeline->OnSongLoaded();
		SetPlaybackTime(previousPlaybackTime);
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
