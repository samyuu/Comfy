#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "TargetInspector.h"
#include "BPMCalculatorWindow.h"
#include "ChartPropertiesWindow.h"
#include "Timeline/TargetTimeline.h"
#include "RenderWindow/TargetRenderWindow.h"
#include "Undo/Undo.h"
#include "ImGui/Widgets/FileViewer.h"
#include "Editor/Common/UndoHistoryWindow.h"

namespace Comfy::Studio::Editor
{
	class ChartEditor : public IEditorComponent
	{
	public:
		ChartEditor(Application& parent, EditorManager& editor);
		~ChartEditor() = default;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;
		void GuiMenu() override;

	public:
		bool IsAudioFile(std::string_view filePath);
		void OpenLoadAudioFileDialog();

		bool OnFileDropped(const std::string& filePath) override;

		bool LoadSongAsync(std::string_view filePath);
		void UnloadSong();

		void CreateNewChart();
		void LoadChartFileSync(std::string_view filePath);
		void SaveChartFileAsync(std::string_view filePath = "");

		void OpenReadChartFileDialog();
		void OpenSaveChartFileDialog();

		bool IsSongAsyncLoading() const;

		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();

		bool GetIsPlayback() const;

		Audio::SourceHandle GetSongSource();
		Audio::Voice GetSongVoice();

		TimeSpan GetPlaybackTimeAsync() const;
		void SetPlaybackTime(TimeSpan value);

		TimeSpan GetPlaybackTimeOnPlaybackStart() const;

	private:
		void UpdateAsyncSongSourceLoading();

	private:
		Gui::FileViewer songFileViewer = { "dev_ram/sound/song" };

		std::unique_ptr<Chart> chart = nullptr;
		std::unique_ptr<Render::Renderer2D> renderer = nullptr;

		Undo::UndoManager undoManager = {};
		std::unique_ptr<TargetTimeline> timeline;
		std::unique_ptr<TargetRenderWindow> renderWindow;
		SyncWindow syncWindow = { undoManager };
		TargetInspector inspector = { undoManager };
		UndoHistoryWindow historyWindow = { undoManager };
		BPMCalculatorWindow bpmCalculatorWindow = { undoManager };
		ChartPropertiesWindow chartPropertiesWindow = { *this, undoManager };

		std::future<Audio::SourceHandle> songSourceFuture;
		std::string songSourceFilePath;

		Audio::SourceHandle songSource = Audio::SourceHandle::Invalid;
		Audio::Voice songVoice = Audio::VoiceHandle::Invalid;

		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart;
	};
}
