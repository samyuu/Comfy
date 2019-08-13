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
		songInstance = MakeRefPtr<AudioInstance>(&dummySampleProvider, "ChartEditor::SongInstance");
		songInstance->SetPlayPastEnd(true);
		songInstance->SetVolume(0.75f);

		AudioEngine* audioEngine = AudioEngine::GetInstance();
		audioEngine->AddAudioInstance(songInstance);

		timeline->Initialize();
		syncWindow->Initialize();
		renderWindow->Initialize();
	}

	void ChartEditor::DrawGui()
	{
		ImGui::GetCurrentWindow()->Hidden = true;

		if (ImGui::Begin(ICON_FA_FOLDER "  Song Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("SongLoaderChild##ChartEditor");
			if (songFileViewer.DrawGui())
			{
				if (IsAudioFile(songFileViewer.GetFileToOpen()))
					LoadSong(songFileViewer.GetFileToOpen());
			}
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin(ICON_FA_MUSIC "  Target Timeline##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			timeline->DrawTimelineGui();
		}
		ImGui::End();

		if (ImGui::Begin(ICON_FA_SYNC "  Sync Window##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("SyncWindowChild##ChartEditor", ImVec2(0, 0), true);
			syncWindow->DrawGui(chart.get(), timeline.get());
			ImGui::EndChild();
		}
		ImGui::End();

		RenderWindowBase::PushWindowPadding();
		if (ImGui::Begin(ICON_FA_CHART_BAR "  Target Window##ChartEditor", nullptr, ImGuiWindowFlags_None))
		{
			//renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			//renderWindow->SetIsPlayback(timeline->GetIsPlayback());
			//renderWindow->SetCurrentFrame(timeline->GetFrame().Frames());
			renderWindow->DrawGui();
		}
		ImGui::End();
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
		TimeSpan playbackTime = GetPlaybackTime();
		{
			RefPtr<MemoryAudioStream> newSongStream = MakeRefPtr<MemoryAudioStream>(filePath);

			songInstance->SetSampleProvider(newSongStream.get());
			chart->SetDuration(songInstance->GetDuration());

			if (songStream != nullptr)
				songStream->Dispose();

			songStream = newSongStream;

			timeline->OnSongLoaded();
		}
		SetPlaybackTime(playbackTime);
		return true;
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

	MemoryAudioStream* ChartEditor::GetSongStream()
	{
		return songStream.get();
	}

	AudioInstance* ChartEditor::GetSongInstance()
	{
		return songInstance.get();
	}
}