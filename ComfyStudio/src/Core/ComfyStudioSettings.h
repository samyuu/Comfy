#pragma once
#include "Types.h"
#include "Core/SemanticVersion.h"
#include "Editor/Common/RecentFilesList.h"
#include "Editor/Chart/TargetPropertyPresets.h"
#include "Editor/Chart/BPMCalculatorWindow.h"
#include "Editor/Chart/Timeline/TargetTimeline.h"
#include "Editor/Chart/Gameplay/PlayTestCore.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	enum class GameTheme : u8;
	enum class ChartBackgroundDisplayType : u8;
}

namespace Comfy::Studio
{
	constexpr std::string_view ComfyStudioAppSettingsFilePath = "settings_app.json";
	constexpr std::string_view ComfyStudioUserSettingsFilePath = "settings_user.json";

	// NOTE: Loaded at startup and saved on exit
	struct ComfyStudioAppSettings
	{
		static constexpr SemanticVersion CurrentVersion = { 1, 4, 0 };

		bool LoadFromFile(std::string_view filePath = ComfyStudioAppSettingsFilePath);
		void SaveToFile(std::string_view filePath = ComfyStudioAppSettingsFilePath) const;
		void RestoreDefault();

		struct
		{
			std::optional<ivec4> RestoreRegion;
			std::optional<ivec2> Position;
			std::optional<ivec2> Size;
			std::optional<bool> IsFullscreen;
			std::optional<bool> IsMaximized;
			std::optional<i32> SwapInterval;
			std::optional<std::string> ActiveEditorComponent;
		} LastSessionWindowState;

		struct
		{
			std::optional<i32> ExportFormatIndex;
			std::optional<i32> PVID;
			std::optional<std::string> RootDirectory;
			std::optional<std::string> MDataID;
			std::optional<f32> BackgroundDim;
			std::optional<bool> MergeWithExistingMData;
			std::optional<bool> CreateSprSelPV;
			std::optional<bool> AddDummyMovieReference;
			std::optional<f32> VorbisVBRQuality;
		} LastPVScriptExportOptions;

		struct
		{
			Editor::RecentFilesList ChartFiles;
		} RecentFiles;
	};

	// NOTE: Loaded at startup but only saved when manually edited by the user via a settings window
	struct ComfyStudioUserSettings
	{
		static constexpr SemanticVersion CurrentVersion = { 1, 30, 0 };

		bool LoadFromFile(std::string_view filePath = ComfyStudioUserSettingsFilePath);
		void SaveToFile(std::string_view filePath = ComfyStudioUserSettingsFilePath) const;
		void RestoreDefault();

		// NOTE: Explicit mutable accessor to make the syntax very clear. Changes then need to be explicitly written back to disk
		inline ComfyStudioUserSettings& Mutable() const { return *const_cast<ComfyStudioUserSettings*>(this); }

		struct
		{
			struct
			{
				f32 SongVolume;
				f32 ButtonSoundVolume;
				f32 SoundEffectVolume;
				f32 MetronomeVolume;

				bool OpenDeviceOnStartup;
				bool CloseDeviceOnIdleFocusLoss;
				bool RequestExclusiveDeviceAccess;
			} Audio;

			struct
			{
				bool EnableColorCorrectionEditor;
				bool EnableColorCorrectionPlaytest;
				struct
				{
					f32 Gamma;
					f32 Contrast;
					std::array<vec3, 3> ColorCoefficientsRGB;
				} ColorCorrectionParam;
			} Video;

			struct
			{
				bool ShowTestMenu;
				// NOTE: Usually part of the style but exposed as a user setting in case of performance issues
				bool AntiAliasedLines;
				bool AntiAliasedFill;
				i32 TargetDistanceGuideCircleSegments;
				i32 TargetDistanceGuideMaxCount;
				i32 TargetButtonPathCurveSegments;
				i32 TargetButtonPathMaxCount;
			} Gui;

			struct
			{
				bool EnableRichPresence;
				bool ShareElapsedTime;
			} Discord;
		} System;

		struct
		{
			// TODO: Do these make sense (?)
			// bool UnsavedChangesWarningDialog;
			// bool RememberRecentFiles;

			bool AutoSaveEnabled;
			TimeSpan AutoSaveInterval;
			i32 MaxAutoSaveFiles;
			std::string RelativeAutoSaveDirectory;
		} SaveAndLoad;

		struct // NOTE: Underscores in symbols are usually a big no go but definitely help with readability here quite a lot
		{
			Input::StandardControllerLayoutMappings ControllerLayoutMappings;

			Input::MultiBinding App_ToggleFullscreen;
			Input::MultiBinding App_Dialog_YesOrOk;
			Input::MultiBinding App_Dialog_No;
			Input::MultiBinding App_Dialog_Cancel;
			Input::MultiBinding App_Dialog_SelectNextTab;
			Input::MultiBinding App_Dialog_SelectPreviousTab;

			Input::MultiBinding ChartEditor_ChartNew;
			Input::MultiBinding ChartEditor_ChartOpen;
			Input::MultiBinding ChartEditor_ChartSave;
			Input::MultiBinding ChartEditor_ChartSaveAs;
			Input::MultiBinding ChartEditor_ChartOpenDirectory;
			Input::MultiBinding ChartEditor_ImportUPDCChart;
			Input::MultiBinding ChartEditor_ImportPVScriptChart;
			Input::MultiBinding ChartEditor_ExportUPDCChart;
			Input::MultiBinding ChartEditor_ExportPVScriptMData;
			Input::MultiBinding ChartEditor_ExportPVScriptChart;
			Input::MultiBinding ChartEditor_Undo;
			Input::MultiBinding ChartEditor_Redo;
			Input::MultiBinding ChartEditor_OpenSettings;
			Input::MultiBinding ChartEditor_StartPlaytestFromStart;
			Input::MultiBinding ChartEditor_StartPlaytestFromCursor;
			Input::MultiBinding ChartEditor_CreateManualAutoSave;
			Input::MultiBinding ChartEditor_OpenAutoSaveDirectory;

			Input::MultiBinding Timeline_CenterCursor;
			Input::MultiBinding Timeline_TogglePlayback;
			Input::MultiBinding Timeline_StopPlayback;

			Input::MultiBinding TargetTimeline_Cut;
			Input::MultiBinding TargetTimeline_Copy;
			Input::MultiBinding TargetTimeline_Paste;
			Input::MultiBinding TargetTimeline_MoveCursorLeft;
			Input::MultiBinding TargetTimeline_MoveCursorRight;
			Input::MultiBinding TargetTimeline_GoToStartOfTimeline;
			Input::MultiBinding TargetTimeline_GoToEndOfTimeline;
			Input::MultiBinding TargetTimeline_IncreaseGridPrecision;
			Input::MultiBinding TargetTimeline_DecreaseGridPrecision;
			Input::MultiBinding TargetTimeline_SetGridDivision_4;
			Input::MultiBinding TargetTimeline_SetGridDivision_8;
			Input::MultiBinding TargetTimeline_SetGridDivision_12;
			Input::MultiBinding TargetTimeline_SetGridDivision_16;
			Input::MultiBinding TargetTimeline_SetGridDivision_24;
			Input::MultiBinding TargetTimeline_SetGridDivision_32;
			Input::MultiBinding TargetTimeline_SetGridDivision_48;
			Input::MultiBinding TargetTimeline_SetGridDivision_64;
			Input::MultiBinding TargetTimeline_SetGridDivision_96;
			Input::MultiBinding TargetTimeline_SetGridDivision_192;
			Input::MultiBinding TargetTimeline_SetChainSlideGridDivision_12;
			Input::MultiBinding TargetTimeline_SetChainSlideGridDivision_16;
			Input::MultiBinding TargetTimeline_SetChainSlideGridDivision_24;
			Input::MultiBinding TargetTimeline_SetChainSlideGridDivision_32;
			Input::MultiBinding TargetTimeline_SetChainSlideGridDivision_48;
			Input::MultiBinding TargetTimeline_SetChainSlideGridDivision_64;
			Input::MultiBinding TargetTimeline_StartEndRangeSelection;
			Input::MultiBinding TargetTimeline_DeleteSelection;
			Input::MultiBinding TargetTimeline_SelectAll;
			Input::MultiBinding TargetTimeline_DeselectAll;
			Input::MultiBinding TargetTimeline_IncreasePlaybackSpeed;
			Input::MultiBinding TargetTimeline_DecreasePlaybackSpeed;
			Input::MultiBinding TargetTimeline_SetPlaybackSpeed_100Percent;
			Input::MultiBinding TargetTimeline_SetPlaybackSpeed_75Percent;
			Input::MultiBinding TargetTimeline_SetPlaybackSpeed_50Percent;
			Input::MultiBinding TargetTimeline_SetPlaybackSpeed_25Percent;
			Input::MultiBinding TargetTimeline_ToggleMetronome;
			Input::MultiBinding TargetTimeline_ToggleTargetHolds;
			Input::MultiBinding TargetTimeline_PlaceTriangle;
			Input::MultiBinding TargetTimeline_PlaceSquare;
			Input::MultiBinding TargetTimeline_PlaceCross;
			Input::MultiBinding TargetTimeline_PlaceCircle;
			Input::MultiBinding TargetTimeline_PlaceSlideL;
			Input::MultiBinding TargetTimeline_PlaceSlideR;
			Input::MultiBinding TargetTimeline_ModifyTargetsMirrorTypes;
			Input::MultiBinding TargetTimeline_ModifyTargetsExpandTime2To1;
			Input::MultiBinding TargetTimeline_ModifyTargetsExpandTime3To2;
			Input::MultiBinding TargetTimeline_ModifyTargetsExpandTime4To3;
			Input::MultiBinding TargetTimeline_ModifyTargetsCompressTime1To2;
			Input::MultiBinding TargetTimeline_ModifyTargetsCompressTime2To3;
			Input::MultiBinding TargetTimeline_ModifyTargetsCompressTime3To4;
			Input::MultiBinding TargetTimeline_RefineSelectionSelectEvery2ndTarget;
			Input::MultiBinding TargetTimeline_RefineSelectionSelectEvery3rdTarget;
			Input::MultiBinding TargetTimeline_RefineSelectionSelectEvery4thTarget;
			Input::MultiBinding TargetTimeline_RefineSelectionShiftSelectionLeft;
			Input::MultiBinding TargetTimeline_RefineSelectionShiftSelectionRight;
			Input::MultiBinding TargetTimeline_RefineSelectionSelectAllSingleTargets;
			Input::MultiBinding TargetTimeline_RefineSelectionSelectAllSyncTargets;
			Input::MultiBinding TargetTimeline_RefineSelectionSelectAllPartiallySelectedSyncPairs;

			Input::MultiBinding TargetPreview_JumpToPreviousTarget;
			Input::MultiBinding TargetPreview_JumpToNextTarget;
			Input::MultiBinding TargetPreview_TogglePlayback;
			Input::MultiBinding TargetPreview_SelectPositionTool;
			Input::MultiBinding TargetPreview_SelectPathTool;
			Input::MultiBinding TargetPreview_PositionTool_MoveUp;
			Input::MultiBinding TargetPreview_PositionTool_MoveLeft;
			Input::MultiBinding TargetPreview_PositionTool_MoveDown;
			Input::MultiBinding TargetPreview_PositionTool_MoveRight;
			Input::MultiBinding TargetPreview_PositionTool_FlipHorizontal;
			Input::MultiBinding TargetPreview_PositionTool_FlipHorizontalLocal;
			Input::MultiBinding TargetPreview_PositionTool_FlipVertical;
			Input::MultiBinding TargetPreview_PositionTool_FlipVerticalLocal;
			Input::MultiBinding TargetPreview_PositionTool_PositionInRow;
			Input::MultiBinding TargetPreview_PositionTool_PositionInRowBack;
			Input::MultiBinding TargetPreview_PositionTool_InterpolateLinear;
			Input::MultiBinding TargetPreview_PositionTool_InterpolateCircular;
			Input::MultiBinding TargetPreview_PositionTool_InterpolateCircularFlip;
			Input::MultiBinding TargetPreview_PositionTool_StackPositions;
			Input::MultiBinding TargetPreview_PathTool_InvertFrequencies;
			Input::MultiBinding TargetPreview_PathTool_InterpolateAnglesClockwise;
			Input::MultiBinding TargetPreview_PathTool_InterpolateAnglesCounterclockwise;
			Input::MultiBinding TargetPreview_PathTool_InterpolateDistances;
			Input::MultiBinding TargetPreview_PathTool_ApplyAngleIncrementsPositive;
			Input::MultiBinding TargetPreview_PathTool_ApplyAngleIncrementsPositiveBack;
			Input::MultiBinding TargetPreview_PathTool_ApplyAngleIncrementsNegative;
			Input::MultiBinding TargetPreview_PathTool_ApplyAngleIncrementsNegativeBack;

			Input::MultiBinding BPMCalculator_Tap;
			Input::MultiBinding BPMCalculator_Reset;

			Input::MultiBinding Playtest_ReturnToEditorCurrent;
			Input::MultiBinding Playtest_ReturnToEditorPrePlaytest;
			Input::MultiBinding Playtest_ToggleAutoplay;
			Input::MultiBinding Playtest_TogglePause;
			Input::MultiBinding Playtest_RestartFromResetPoint;
			Input::MultiBinding Playtest_MoveResetPointBackward;
			Input::MultiBinding Playtest_MoveResetPointForward;

			std::vector<Editor::PlayTestInputBinding> PlaytestBindings;
		} Input;

		struct
		{
			f32 MouseWheelScrollDirection;
			f32 MouseWheelScrollSpeed;
			f32 MouseWheelScrollSpeedShift;
			f32 PlaybackMouseWheelScrollFactor;
			f32 PlaybackMouseWheelScrollFactorShift;

			f32 PlaybackAutoScrollCursorPositionFactor;

			TimeSpan PlaybackCursorPlacementOffsetWasapiShared;
			TimeSpan PlaybackCursorPlacementOffsetWasapiExclusive;

			f32 SmoothScrollSpeedSec;

			bool ShowStartEndMarkersSong;
			bool ShowStartEndMarkersMovie;

			Editor::TargetTimelineScalingBehavior ScalingBehavior;
			struct
			{
				f32 MinRowHeight;
				f32 MaxRowHeight;
			} ScalingBehaviorAutoFit;
			struct
			{
				f32 IconScale;
				f32 RowHeight;
			} ScalingBehaviorFixedSize;

			Editor::TargetTimelineCursorScrubbingEdgeAutoScrollThreshold CursorScrubbingEdgeAutoScrollThreshold;
			struct
			{
				f32 Pixels;
			} CursorScrubbingEdgeAutoScrollThresholdFixedSize;
			struct
			{
				f32 Factor;
			} CursorScrubbingEdgeAutoScrollThresholdProportional;
			f32 CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec;
			f32 CursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift;
		} TargetTimeline;

		struct
		{
			bool ShowButtons;
			bool ShowHoldInfo;
			Editor::BeatTick PostHitLingerDuration;
		} TargetPreview;

		struct
		{
			bool ShowDistanceGuides;
			bool ShowTargetGrabTooltip;
			bool UseAxisSnapGuides;
			f32 AxisSnapGuideDistanceThreshold;
			f32 PositionMouseSnap;
			f32 PositionMouseSnapRough;
			f32 PositionMouseSnapPrecise;
			f32 PositionKeyMoveStep;
			f32 PositionKeyMoveStepRough;
			f32 PositionKeyMoveStepPrecise;
			f32 PositionInterpolationCommandSnap;
			f32 MouseRowCenterDistanceThreshold;
			std::vector<Editor::Rules::DiagonalRowLayoutUserDefinition> DiagonalMouseRowLayouts;
		} PositionTool;

		struct
		{
			f32 AngleMouseSnap;
			f32 AngleMouseSnapRough;
			f32 AngleMouseSnapPrecise;
			f32 AngleMouseScrollDirection;
			f32 AngleMouseScrollStep;
			f32 AngleMouseScrollRough;
			f32 AngleMouseScrollPrecise;
			f32 AngleMouseMovementDistanceThreshold;
			f32 AngleMouseTargetCenterDistanceThreshold;
		} PathTool;

		struct
		{
			std::vector<Editor::StaticSyncPreset> StaticSyncPresets;
			std::vector<Editor::SequencePreset> SequencePresets;

			struct
			{
				std::vector<f32> PositionsX;
				std::vector<f32> PositionsY;
				std::vector<f32> Angles;
				std::vector<i32> Frequencies;
				std::vector<f32> Amplitudes;
				std::vector<f32> Distances;
			} InspectorDropdown;
		} TargetPreset;

		struct
		{
			std::string ChartCreatorDefaultName;
		} ChartProperties;

		struct
		{
			bool AutoResetEnabled;
			bool ApplyToTempoMap;
			Editor::BPMTapSoundType TapSoundType;
		} BPMCalculator;

		struct
		{
			bool EnterFullscreenOnMaximizedStart;
			bool AutoHideCursor;
			TimeSpan SongOffsetWasapiShared;
			TimeSpan SongOffsetWasapiExclusive;
		} Playtest;

		struct
		{
			Editor::GameTheme Theme;

			struct
			{
				Editor::ChartBackgroundDisplayType Editor;
				Editor::ChartBackgroundDisplayType EditorWithMovie;
				Editor::ChartBackgroundDisplayType Playtest;
				Editor::ChartBackgroundDisplayType PlaytestWithMovie;
			} BackgroundDisplayType;

			struct
			{
				bool ShowHorizontalSyncMarkers;
			} PlacementGrid;

			struct
			{
				vec4 PrimaryColor;
				vec4 SecondaryColor;
				i32 ScreenPixelSize;
			} CheckerboardBackground;

			struct
			{
				bool DisableBackgroundGrid;
				bool DisableBackgroundDim;
				bool HideNowPrintingPlaceholderImages;
				bool HideCoverImage;
				bool HideLogoImage;
			} PracticeBackground;

			struct
			{
				vec4 OverlayColor;
				vec4 OverlayColorGridAndPractice;
				vec4 PreStartPostEndColor;
			} MovieBackground;
		} Interface;
	};

	// NOTE: Changes are always saved so no need for const protection or manual saves
	inline ComfyStudioAppSettings GlobalAppData = {};

	// NOTE: Global variable for cleaner syntax but const to avoid accidental changes outside an explicit settings window without saving back to disk.
	//		 Modifications should then be done via the Mutable() accessor method.
	inline const ComfyStudioUserSettings GlobalUserData = {};
}
