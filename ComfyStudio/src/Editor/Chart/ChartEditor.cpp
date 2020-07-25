#include "ChartEditor.h"
#include "Misc/StringUtil.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	ChartEditor::ChartEditor(Application& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		chart = std::make_unique<Chart>();

		timeline = std::make_unique<TargetTimeline>(*this);
		syncWindow = std::make_unique<SyncWindow>();
		renderWindow = std::make_unique<TargetRenderWindow>();
	}

	void ChartEditor::OnFirstFrame()
	{
		songVoice = Audio::Engine::GetInstance().AddVoice(Audio::SourceHandle::Invalid, "ChartEditor::SongVoice", false, 0.75f, true);

		timeline->Initialize();
		syncWindow->OnFirstFrame();
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
					LoadSong(songFileViewer.GetFileToOpen());
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
			Gui::BeginChild("SyncWindowChild##ChartEditor", vec2(0.0f, 0.0f), true);
			syncWindow->Gui(*chart, *timeline);
			Gui::EndChild();
		}
		Gui::End();

		// renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
		// renderWindow->SetIsPlayback(timeline->GetIsPlayback());
		// renderWindow->SetCurrentFrame(timeline->GetFrame().Frames());
		renderWindow->BeginEndGui(ICON_FA_CHART_BAR "  Target Window##ChartEditor");
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
		return (IsAudioFile(filePath) && LoadSong(filePath));
	}

	bool ChartEditor::LoadSong(std::string_view filePath)
	{
		songSourceFuture = Audio::Engine::GetInstance().LoadAudioSourceAsync(filePath);
		return true;
	}

	TimeSpan ChartEditor::GetPlaybackTimeAsync() const
	{
		return songVoice.GetPosition() - chart->GetStartOffset();
	}

	void ChartEditor::SetPlaybackTime(TimeSpan value)
	{
		songVoice.SetPosition(value + chart->GetStartOffset());
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

		Audio::Engine::GetInstance().UnloadSource(songSource);

		songVoice.SetSource(newSongStream);
		songSource = newSongStream;
		chart->SetDuration(songVoice.GetDuration());

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
