#pragma once
#include "Editor/IEditorComponent.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "Timeline/TargetTimeline.h"
#include "TargetRenderWindow.h"
#include "Audio/DummySampleProvider.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Editor
{
	class ChartEditor : public IEditorComponent
	{
	public:
		ChartEditor(Application* parent, EditorManager* editor);
		~ChartEditor();

		virtual void Initialize() override;
		virtual void DrawGui() override;
		virtual const char* GetGuiName() const override;
		virtual ImGuiWindowFlags GetWindowFlags() const override;

		bool IsAudioFile(const std::string& filePath);
		virtual bool OnFileDropped(const std::string& filePath) override;

		bool LoadSong(const std::string& filePath);

		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();

		bool GetIsPlayback() const;

		MemoryAudioStream* GetSongStream();
		AudioInstance* GetSongInstance();

		TimeSpan GetPlaybackTime() const;
		void SetPlaybackTime(TimeSpan value);

		Chart* GetChart();
		TimeSpan GetPlaybackTimeOnPlaybackStart() const;

	private:
		std::array<const char*, 4> audioFileExtensions = { ".wav", ".flac", ".ogg", ".mp3" };
		ImGui::FileViewer songFileViewer = { "dev_ram/sound/song" };

		std::unique_ptr<Chart> chart;

		std::unique_ptr<TargetTimeline> timeline;
		std::unique_ptr<SyncWindow> syncWindow;
		std::unique_ptr<TargetRenderWindow> renderWindow;

		DummySampleProvider dummySampleProvider;
		std::shared_ptr<MemoryAudioStream> songStream;
		std::shared_ptr<AudioInstance> songInstance;
		
		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart;
	};
}