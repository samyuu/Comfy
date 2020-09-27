#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "TargetInspector.h"
#include "BPMCalculatorWindow.h"
#include "ChartPropertiesWindow.h"
#include "ChartEditorManual.h"
#include "FileFormat/ComfyStudioChartFile.h"
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
		void OnEditorComponentMadeActive() override;
		ApplicationHostCloseResponse OnApplicationClosing() override;

	public:
		bool IsAudioFile(std::string_view filePath);
		bool OpenLoadAudioFileDialog();

		bool OnFileDropped(std::string_view filePath) override;

		void LoadSongAsync(std::string_view filePath);
		void UnloadSong();

		void CreateNewChart();
		void LoadChartFileSync(std::string_view filePath);
		void SaveChartFileAsync(std::string_view filePath = "");

		bool OpenReadChartFileDialog();
		bool OpenSaveChartFileDialog();
		bool TrySaveChartFileOrOpenDialog();

		void ImportChartFileSync(std::string_view filePath);
		void ExportChartFileSync(std::string_view filePath);

		bool OpenReadImportChartFileDialog();
		bool OpenSaveExportChartFileDialog();

		void CheckOpenSaveConfirmationPopupThenCall(std::function<void()> onSuccess);

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
		void UpdateApplicationClosingRequest();
		void UpdateGlobalControlInput();
		void UpdateApplicationWindowTitle();
		void UpdateAsyncSongSourceLoading();

		void GuiSubWindows();
		void GuiSaveConfirmationPopup();

	private:
		bool applicationExitRequested = false;
		std::string windowTitle, lastSetWindowTitle;

		std::unique_ptr<Chart> chart = nullptr;
		std::unique_ptr<Render::Renderer2D> renderer = nullptr;

	private:
		Undo::UndoManager undoManager = {};
		std::unique_ptr<TargetTimeline> timeline;
		std::unique_ptr<TargetRenderWindow> renderWindow;
		SyncWindow syncWindow = { undoManager };
		TargetInspector inspector = { undoManager };
		UndoHistoryWindow historyWindow = { undoManager };
		BPMCalculatorWindow bpmCalculatorWindow = { undoManager };
		ChartPropertiesWindow chartPropertiesWindow = { *this, undoManager };
		ChartEditorManual chartEditorManual = { *this };

	private:
		struct SaveConfirmationPopupData
		{
			bool OpenOnNextFrame;
			std::function<void()> OnSuccessFunction;
		} saveConfirmationPopup = {};

		std::future<bool> chartSaveFileFuture;
		std::unique_ptr<ComfyStudioChartFile> lastSavedChartFile;

	private:
		std::string songSourceFilePath;
		std::future<Audio::SourceHandle> songSourceFuture;

		Audio::SourceHandle songSource = Audio::SourceHandle::Invalid;
		Audio::Voice songVoice = Audio::VoiceHandle::Invalid;

	private:
		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart;
	};
}
