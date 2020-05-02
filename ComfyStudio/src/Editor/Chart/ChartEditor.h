#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "Timeline/TargetTimeline.h"
#include "TargetRenderWindow.h"
#include "Audio/SampleProvider/SilenceSampleProvider.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Comfy::Editor
{
	class ChartEditor : public IEditorComponent
	{
	public:
		ChartEditor(Application* parent, EditorManager* editor);
		~ChartEditor();

		void Initialize() override;
		void DrawGui() override;
		const char* GetGuiName() const override;
		ImGuiWindowFlags GetWindowFlags() const override;

		bool IsAudioFile(std::string_view filePath);
		bool OnFileDropped(const std::string& filePath) override;

		bool LoadSong(std::string_view filePath);

		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();

		bool GetIsPlayback() const;

		Audio::MemorySampleProvider* GetSongStream();
		Audio::AudioInstance* GetSongInstance();

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

		std::shared_ptr<Audio::SilenceSampleProvider> dummySampleProvider;
		std::shared_ptr<Audio::MemorySampleProvider> songStream;
		std::shared_ptr<Audio::AudioInstance> songInstance;
		
		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart;
	};
}
