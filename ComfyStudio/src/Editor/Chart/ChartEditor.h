#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "Timeline/TargetTimeline.h"
#include "TargetRenderWindow.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Comfy::Studio::Editor
{
	class ChartEditor : public IEditorComponent
	{
	public:
		ChartEditor(Application& parent, EditorManager& editor);
		~ChartEditor() = default;

	public:
		void OnFirstFrame() override;
		
	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	public:
		bool IsAudioFile(std::string_view filePath);
		bool OnFileDropped(const std::string& filePath) override;

		bool LoadSong(std::string_view filePath);

		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();

		bool GetIsPlayback() const;

		Audio::SourceHandle GetSongSource();
		Audio::Voice GetSongVoice();

		TimeSpan GetPlaybackTime() const;
		void SetPlaybackTime(TimeSpan value);

		Chart* GetChart();
		TimeSpan GetPlaybackTimeOnPlaybackStart() const;

	private:
		std::array<const char*, 4> audioFileExtensions = { ".wav", ".flac", ".ogg", ".mp3" };
		Gui::FileViewer songFileViewer = { "dev_ram/sound/song" };

		std::unique_ptr<Chart> chart;

		std::unique_ptr<TargetTimeline> timeline;
		std::unique_ptr<SyncWindow> syncWindow;
		std::unique_ptr<TargetRenderWindow> renderWindow;

		Audio::SourceHandle songSource = Audio::SourceHandle::Invalid;
		Audio::Voice songVoice = Audio::VoiceHandle::Invalid;

		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart;
	};
}
