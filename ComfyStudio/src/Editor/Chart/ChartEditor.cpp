#include "ChartEditor.h"
#include "Misc/StringHelper.h"
#include <FontIcons.h>

namespace Editor
{
	ChartEditor::ChartEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		chart = MakeUnique<Chart>();

		timeline = MakeUnique<TargetTimeline>(this);
		syncWindow = MakeUnique<SyncWindow>();
		renderWindow = MakeUnique<TargetRenderWindow>();
	}

	ChartEditor::~ChartEditor()
	{
	}

	void ChartEditor::Initialize()
	{
		dummySampleProvider = MakeRef<Audio::SilenceSampleProvider>();

		songInstance = MakeRef<Audio::AudioInstance>(dummySampleProvider, true, "ChartEditor::SongInstance");
		songInstance->SetPlayPastEnd(true);
		songInstance->SetVolume(0.75f);

		Audio::AudioEngine* audioEngine = Audio::AudioEngine::GetInstance();
		audioEngine->AddAudioInstance(songInstance);

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

	bool ChartEditor::IsAudioFile(const std::string& filePath)
	{
		for (auto& fileExtension : audioFileExtensions)
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

	bool ChartEditor::LoadSong(const std::string& filePath)
	{
		bool success;

		TimeSpan playbackTime = GetPlaybackTime();
		{
			RefPtr<Audio::MemorySampleProvider> newSongStream = Audio::AudioEngine::GetInstance()->LoadAudioFile(filePath);
			success = newSongStream != nullptr;

			if (success)
			{
				songInstance->SetSampleProvider(newSongStream);
				songStream = newSongStream;
				chart->SetDuration(songInstance->GetDuration());
			}
			else
			{
				songInstance->SetSampleProvider(dummySampleProvider);
			}

			timeline->OnSongLoaded();
		}
		SetPlaybackTime(playbackTime);

		return success;
	}

	TimeSpan ChartEditor::GetPlaybackTime() const
	{
		return songInstance->GetPosition() - chart->GetStartOffset();
	}

	void ChartEditor::SetPlaybackTime(TimeSpan value)
	{
		songInstance->SetPosition(value + chart->GetStartOffset());
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
		songInstance->SetIsPlaying(true);

		timeline->OnPlaybackResumed();
	}

	void ChartEditor::PausePlayback()
	{
		songInstance->SetIsPlaying(false);
		isPlaying = false;

		timeline->OnPlaybackPaused();
	}

	void ChartEditor::StopPlayback()
	{
		SetPlaybackTime(playbackTimeOnPlaybackStart);
		PausePlayback();

		timeline->OnPlaybackStopped();
	}

	Audio::MemorySampleProvider* ChartEditor::GetSongStream()
	{
		return songStream.get();
	}

	Audio::AudioInstance* ChartEditor::GetSongInstance()
	{
		return songInstance.get();
	}
}