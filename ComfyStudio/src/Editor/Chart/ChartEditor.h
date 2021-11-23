#pragma once
#include "Types.h"
#include "Editor/Core/IEditorComponent.h"
#include "ChartEditorSettingsWindow.h"
#include "Core/ComfyStudioDiscord.h"
#include "Chart.h"
#include "SyncWindow.h"
#include "PresetWindow.h"
#include "TargetInspector.h"
#include "BPMCalculatorWindow.h"
#include "ChartPropertiesWindow.h"
#include "FileFormat/ComfyStudioChartFile.h"
#include "Timeline/TargetTimeline.h"
#include "RenderWindow/TargetRenderWindow.h"
#include "ChartMoviePlaybackController.h"
#include "Gameplay/PlayTestWindow.h"
#include "PVScript/PVScriptImportWindow.h"
#include "PVScript/PVScriptExportWindow.h"
#include "Editor/Common/UndoHistoryWindow.h"
#include "Editor/Common/SoundEffectManager.h"
#include "Editor/Common/RecentFilesList.h"
#include "ImGui/Widgets/FileViewer.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	// TODO: Move these into a different header
	enum class GameTheme : u8
	{
		PS4FutureTone,
		PS4FutureSound,
		PS4ColorfulTone,
		Count
	};

	static constexpr std::array<const char*, EnumCount<GameTheme>()> GameThemeNames =
	{
		"PS4 Future Tone",
		"PS4 Future Sound",
		"PS4 Colorful Tone",
	};

	enum class ChartBackgroundDisplayType : u8
	{
		Checkerboard,
		Checkerboard_Grid,
		Image,
		Image_Grid,
		Image_Practice,

		Movie,
		Movie_Grid,
		Movie_Practice,

		Count,
		FirstNoMovie = Checkerboard,
		LastNoMovie = Image_Practice,
		FirstMovie = Movie,
		LastMovie = Movie_Practice,
	};

	static constexpr std::array<const char*, EnumCount<ChartBackgroundDisplayType>()> ChartBackgroundDisplayTypeNames =
	{
		"Checkerboard",
		"Checkerboard + Grid",
		"Image",
		"Image + Grid",
		"Image + Practice",
		"Movie",
		"Movie + Grid",
		"Movie + Practice",
	};

	constexpr bool HasChartBackgroundDisplayTypeGrid(ChartBackgroundDisplayType type) { return (type == ChartBackgroundDisplayType::Checkerboard_Grid) || (type == ChartBackgroundDisplayType::Movie_Grid) || (type == ChartBackgroundDisplayType::Image_Grid); }
	constexpr bool HasChartBackgroundDisplayTypeCheckerboard(ChartBackgroundDisplayType type) { return (type == ChartBackgroundDisplayType::Checkerboard) || (type == ChartBackgroundDisplayType::Checkerboard_Grid); }
	constexpr bool HasChartBackgroundDisplayTypeImage(ChartBackgroundDisplayType type) { return (type == ChartBackgroundDisplayType::Image) || (type == ChartBackgroundDisplayType::Image_Grid) || (type == ChartBackgroundDisplayType::Image_Practice); }
	constexpr bool HasChartBackgroundDisplayTypePractice(ChartBackgroundDisplayType type) { return (type == ChartBackgroundDisplayType::Image_Practice) || (type == ChartBackgroundDisplayType::Movie_Practice); }
	constexpr bool HasChartBackgroundDisplayTypeMovie(ChartBackgroundDisplayType type) { return (type == ChartBackgroundDisplayType::Movie) || (type == ChartBackgroundDisplayType::Movie_Grid) || (type == ChartBackgroundDisplayType::Movie_Practice); }

	class ChartEditor : public IEditorComponent
	{
	public:
		ChartEditor(ComfyStudioApplication& parent, EditorManager& editor);
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
		bool OpenLoadMovieFileDialog();

		bool OnFileDropped(std::string_view filePath) override;

		void LoadSongAsync(std::string_view filePath);
		void UnloadSong();

		void LoadMovieAsync(std::string_view filePath);
		void UnloadMovie(bool disposeMoviePlayer = false);

		void CreateNewChart(bool focusTimelineAndCloseActivePopup = true);
		void LoadNativeChartFileSync(std::string_view filePath);
		void SaveNativeChartFileAsync(std::string_view filePath = "");

		void OpenChartDirectoryInExplorer() const;
		void OpenAutoSaveDirectoryInExplorer() const;
		bool OpenReadNativeChartFileDialog();
		bool OpenSaveNativeChartFileDialog();
		bool TrySaveNativeChartFileOrOpenDialog();

		void ImportPJEChartFileSync(std::string_view filePath);
		void ExportPJEChartFileSync(std::string_view filePath);

		bool OpenReadImportPJEChartFileDialog();
		bool OpenSaveExportPJEChartFileDialog();

		void OpenPVScriptImportWindow(std::string_view filePath);
		bool OpenReadImportPVScriptFileDialogThenOpenImportWindow();

		void OpenPVScriptExportWindow();
		bool OpenSaveExportSimplePVScriptChartFileDialog();

		void CheckOpenSaveConfirmationPopupThenCall(std::function<void()> onSuccess);

		std::string GetOpenReadImageFileDialogPath() const;

		bool IsSongAsyncLoading() const;
		bool IsMovieAsyncLoading() const;

		void ResumePlayback();
		void PausePlayback();
		void StopPlayback();

		bool GetIsPlayback() const;

		Audio::SourceHandle GetSongSource();
		Audio::Voice GetSongVoice();

		TimeSpan GetPlaybackTimeAsync() const;
		void SetPlaybackTime(TimeSpan value);

		TimeSpan GetPlaybackTimeOnPlaybackStart() const;

		std::array<std::optional<TimeSpan>, 2> GetSongAndMovieSourceDurations() const;

		SoundEffectManager& GetSoundEffectManager();
		ChartMoviePlaybackController& GetMoviePlaybackController();

		ComfyStudioApplication& GetParentApplication();

	private:
		std::unique_ptr<Chart> MakeNewChartWithDefaults() const;

		void UpdateApplicationClosingRequest();
		void UpdateGlobalControlInput();
		void UpdateApplicationWindowTitle();
		void UpdateDiscordStatusIfEnabled(bool isPlaytesting);
		void UpdateAsyncSongSourceLoading();
		void CheckAutoSaveStopwatchAndDoAsyncAutoSave();
		void AutoSaveCurrentChartIfEnabledAndRestartStopwatch();
		void StartAsyncAutoSaveFutureForChart(const Chart& chartToSave) const;

		void GuiChildWindows();
		void GuiPlaytestFullscreenFadeOutAnimation();

		void GuiSettingsPopup();
		void GuiPVScriptImportPopup();
		void GuiPVScriptExportPopup();
		void GuiFileNotFoundPopup();
		void GuiSaveConfirmationPopup();

		void SyncWorkingChartPointers();

		PlayTestWindow& GetOrCreatePlayTestWindow();
		void StartPlaytesting(bool startFromCursor);
		void StopPlaytesting(PlayTestExitType exitType, bool focusTimelineAndCloseActivePopup = true, bool startFadeOutAnimation = true);

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
		std::optional<i32> guiFrameCountOnPlaytestExit = {};
		Stopwatch playtestFadeOutStopwatch = {};

	private:
		Undo::UndoManager undoManager = {};
		std::unique_ptr<TargetTimeline> timeline;
		std::unique_ptr<TargetRenderWindow> renderWindow;
		SyncWindow syncWindow = { undoManager };
		PresetWindow presetWindow = { *this, undoManager };
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

		struct PVScriptExportPopupData
		{
			bool OpenOnNextFrame;
			PVScriptExportWindow Window;
		} pvScriptExportPopup = {};

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

		mutable std::future<bool> chartAutoSaveFuture;
		Stopwatch lastAutoSaveStowpatch = {};

#if COMFY_COMILE_WITH_DLL_DISCORD_RICH_PRESENCE_INTEGRATION
		Discord::Status discordStatus = {};
		i64 unixTimeOnChartBegin = 0;
#endif

	private:
		std::string songSourceFilePathAbsolute;
		std::future<Audio::SourceHandle> songSourceFuture;

		Audio::SourceHandle songSource = Audio::SourceHandle::Invalid;
		Audio::Voice songVoice = Audio::VoiceHandle::Invalid;

		std::string movieFilePathAbsolute;
		std::unique_ptr<Render::IMoviePlayer> moviePlayer = nullptr;

		ChartMoviePlaybackController moviePlaybackController = {};

	private:
		bool isPlaying = false;
		TimeSpan playbackTimeOnPlaybackStart = {};
		TimeSpan playbackTimeOnPlaytestStart = {};
		f32 timelineScrollXOnPlaytestStart = {};
	};
}
