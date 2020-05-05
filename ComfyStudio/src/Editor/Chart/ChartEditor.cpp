#include "ChartEditor.h"
#include "Misc/StringHelper.h"
#include <FontIcons.h>

namespace Comfy::Editor
{
	ChartEditor::ChartEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		chart = std::make_unique<Chart>();

		timeline = std::make_unique<TargetTimeline>(this);
		syncWindow = std::make_unique<SyncWindow>();
		renderWindow = std::make_unique<TargetRenderWindow>();
	}

	ChartEditor::~ChartEditor()
	{
	}

	void ChartEditor::Initialize()
	{
		songVoice = Audio::AudioEngine::GetInstance().AddVoice(Audio::SourceHandle::Invalid, "ChartEditor::SongVoice", false, 0.75f, true);

		timeline->Initialize();
		syncWindow->Initialize();
		renderWindow->Initialize();
	}

	void ChartEditor::DrawGui()
	{
		Gui::GetCurrentWindow()->Hidden = true;

		if (Gui::Begin(ICON_FA_FOLDER "  Song Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			Gui::BeginChild("SongLoaderChild##ChartEditor");
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
			syncWindow->DrawGui(chart.get(), timeline.get());
			Gui::EndChild();
		}
		Gui::End();

		RenderWindowBase::PushWindowPadding();
		if (Gui::Begin(ICON_FA_CHART_BAR "  Target Window##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			//renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			//renderWindow->SetIsPlayback(timeline->GetIsPlayback());
			//renderWindow->SetCurrentFrame(timeline->GetFrame().Frames());
			renderWindow->DrawGui();
		}
		Gui::End();
		RenderWindowBase::PopWindowPadding();
	}

	const char* ChartEditor::GetGuiName() const
	{
		return u8"Chart Editor";
	}

	ImGuiWindowFlags ChartEditor::GetWindowFlags() const
	{
		return BaseWindow::GetNoWindowFlags();
	}

	bool ChartEditor::IsAudioFile(std::string_view filePath)
	{
		for (const auto& fileExtension : audioFileExtensions)
		{
			if (EndsWithInsensitive(filePath, fileExtension))
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
		bool success = false;

		TimeSpan playbackTime = GetPlaybackTime();
		{
			auto newSongStream = Audio::AudioEngine::GetInstance().LoadAudioSource(filePath);
			success = true;

			if (success)
			{
				songVoice.SetSource(newSongStream);
				songStream = newSongStream;
				chart->SetDuration(songVoice.GetDuration());
			}

			timeline->OnSongLoaded();
		}
		SetPlaybackTime(playbackTime);

		return success;
	}

	TimeSpan ChartEditor::GetPlaybackTime() const
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

	bool ChartEditor::GetIsPlayback() const
	{
		return isPlaying;
	}

	void ChartEditor::ResumePlayback()
	{
		playbackTimeOnPlaybackStart = GetPlaybackTime();

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
		return songStream;
	}

	Audio::Voice ChartEditor::GetSongVoice()
	{
		return songVoice;
	}
}
