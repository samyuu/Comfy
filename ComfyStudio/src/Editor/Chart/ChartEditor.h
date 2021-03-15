#pragma once
#include "Types.h"
#include "Editor/Core/IEditorComponent.h"
#include "ChartEditorSettingsWindow.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "PresetWindow.h"
#include "TargetInspector.h"
#include "BPMCalculatorWindow.h"
#include "ChartPropertiesWindow.h"
#include "FileFormat/ComfyStudioChartFile.h"
#include "Timeline/TargetTimeline.h"
#include "RenderWindow/TargetRenderWindow.h"
#include "Gameplay/PlayTestWindow.h"
#include "PVScript/PVScriptImportWindow.h"
#include "Editor/Common/UndoHistoryWindow.h"
#include "Editor/Common/SoundEffectManager.h"
#include "Editor/Common/RecentFilesList.h"
#include "ImGui/Widgets/FileViewer.h"
#include "Undo/Undo.h"

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

		void OnExclusiveGui() override;

	public:
		bool OpenLoadAudioFileDialog();

		bool OnFileDropped(std::string_view filePath) override;

		void LoadSongAsync(std::string_view filePath);
		void UnloadSong();

		void CreateNewChart();
		void LoadNativeChartFileSync(std::string_view filePath);
		void SaveNativeChartFileAsync(std::string_view filePath = "");

		bool OpenReadNativeChartFileDialog();
		bool OpenSaveNativeChartFileDialog();
		bool TrySaveNativeChartFileOrOpenDialog();

		void ImportPJEChartFileSync(std::string_view filePath);
		void ExportPJEChartFileSync(std::string_view filePath);

		bool OpenReadImportPJEChartFileDialog();
		bool OpenSaveExportPJEChartFileDialog();

		void OpenPVScriptImportWindow(std::string_view filePath);
		bool OpenReadImportPVScriptFileDialogThenOpenImportWindow();

		void CheckOpenSaveConfirmationPopupThenCall(std::function<void()> onSuccess);

		std::string GetOpenReadImageFileDialogPath() const;

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

		SoundEffectManager& GetSoundEffectManager();

	private:
		void UpdateApplicationClosingRequest();
		void UpdateGlobalControlInput();
		void UpdateApplicationWindowTitle();
		void UpdateAsyncSongSourceLoading();

		void GuiChildWindows();

		void GuiSettingsPopup();
		void GuiPVScriptImportPopup();
		void GuiFileNotFoundPopup();
		void GuiSaveConfirmationPopup();

		void SyncWorkingChartPointers();

		PlayTestWindow& GetOrCreatePlayTestWindow();
		void StartPlaytesting(bool startFromCursor);
		void StopPlaytesting(PlayTestExitType exitType);

	private:
		bool applicationExitRequested = false;
		std::string windowTitle, lastSetWindowTitle;

		std::unique_ptr<Chart> chart = nullptr;
		std::unique_ptr<Render::Renderer2D> renderer = nullptr;

		std::unique_ptr<Graphics::SprSet> editorSprites = nullptr;

		SoundEffectManager soundEffectManager;
		std::unique_ptr<ButtonSoundController> buttonSoundController = nullptr;

		std::unique_ptr<PlayTestWindow> playTestWindow = nullptr;
		bool exitFullscreenOnPlaytestEnd = false;

	private:
		Undo::UndoManager undoManager = {};
		std::unique_ptr<TargetTimeline> timeline;
		std::unique_ptr<TargetRenderWindow> renderWindow;
		SyncWindow syncWindow = { undoManager };
		PresetWindow presetWindow = { undoManager };
		TargetInspector inspector = { undoManager };
		UndoHistoryWindow historyWindow = { undoManager };
		BPMCalculatorWindow bpmCalculatorWindow = { undoManager };
		ChartPropertiesWindow chartPropertiesWindow = { *this, undoManager };

	private:
		struct SettingsPopupData
		{
			bool OpenOnNextFrame;
			bool WasOpenLastFrame;
			ChartEditorSettingsWindow Window;
		} settingsPopup = {};

		struct PVScriptImportPopupData
		{
			bool OpenOnNextFrame;
			PVScriptImportWindow Window;
		} pvScriptImportPopup = {};

		struct SaveConfirmationPopupData
		{
			bool OpenOnNextFrame;
			std::function<void()> OnSuccessFunction;
		} saveConfirmationPopup = {};

		struct FileNotFoundPopupData
		{
			bool OpenOnNextFrame;
			std::string NotFoundPath;
			std::string_view QuestionToTheUser;
			std::function<void()> OnYesClickedFunction;
		} fileNotFoundPopup = {};

		std::future<bool> chartSaveFileFuture;
		std::unique_ptr<ComfyStudioChartFile> lastSavedChartFile;

	private:
		std::string songSourceFilePathAbsolute;
		std::future<Audio::SourceHandle> songSourceFuture;

		Audio::SourceHandle songSource = Audio::SourceHandle::Invalid;
		Audio::Voice songVoice = Audio::VoiceHandle::Invalid;

	private:
		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart = {};
		TimeSpan playbackTimeOnPlaytestStart = {};
		f32 timelineScrollXOnPlaytestStart = {};
	};
}
