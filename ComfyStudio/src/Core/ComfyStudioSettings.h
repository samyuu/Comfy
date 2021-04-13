#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Core/SemanticVersion.h"
#include "Editor/Common/RecentFilesList.h"
#include "Editor/Chart/TargetPropertyPresets.h"
#include "Editor/Chart/BPMCalculatorWindow.h"
#include "Editor/Chart/Gameplay/PlayTestCore.h"
#include "Input/Input.h"

namespace Comfy::Studio
{
	constexpr std::string_view ComfyStudioAppSettingsFilePath = "settings_app.json";
	constexpr std::string_view ComfyStudioUserSettingsFilePath = "settings_user.json";

	// NOTE: Loaded at startup and saved on exit
	struct ComfyStudioAppSettings
	{
		static constexpr SemanticVersion CurrentVersion = { 1, 1, 0 };

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
			Editor::RecentFilesList ChartFiles;
		} RecentFiles;
	};

	// NOTE: Loaded at startup but only saved when manually edited by the user via a settings window
	struct ComfyStudioUserSettings
	{
		static constexpr SemanticVersion CurrentVersion = { 1, 13, 0 };

		bool LoadFromFile(std::string_view filePath = ComfyStudioUserSettingsFilePath);
		void SaveToFile(std::string_view filePath = ComfyStudioUserSettingsFilePath) const;
		void RestoreDefault();

		// NOTE: Explicit mutable accessor to make the syntax very clear. Changes then need to be explicitly written back to disk
		inline ComfyStudioUserSettings& Mutable() const { return *const_cast<ComfyStudioUserSettings*>(this); }

		struct
		{
			struct
			{
				// Usually render resolution, MSAA settings etc. but that doesn't really make sense for 2D..?
			} Video;

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
				bool ShowTestMenu;
				// NOTE: Usually part of the style but exposed as a user setting in case of performance issues
				bool AntiAliasedLines;
				bool AntiAliasedFill;
				i32 TargetDistanceGuideCircleSegments;
				i32 TargetDistanceGuideMaxCount;
				i32 TargetButtonPathCurveSegments;
				i32 TargetButtonPathMaxCount;
			} Gui;
		} System;

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
			Input::MultiBinding ChartEditor_Undo;
			Input::MultiBinding ChartEditor_Redo;
			Input::MultiBinding ChartEditor_OpenSettings;
			Input::MultiBinding ChartEditor_StartPlaytestFromStart;
			Input::MultiBinding ChartEditor_StartPlaytestFromCursor;

			Input::MultiBinding Timeline_CenterCursor;
			Input::MultiBinding Timeline_TogglePlayback;
			Input::MultiBinding Timeline_StopPlayback;

			Input::MultiBinding TargetTimeline_Cut;
			Input::MultiBinding TargetTimeline_Copy;
			Input::MultiBinding TargetTimeline_Paste;
			Input::MultiBinding TargetTimeline_MoveCursorLeft;
			Input::MultiBinding TargetTimeline_MoveCursorRight;
			Input::MultiBinding TargetTimeline_IncreaseGridPrecision;
			Input::MultiBinding TargetTimeline_DecreaseGridPrecision;
			Input::MultiBinding TargetTimeline_StartEndRangeSelection;
			Input::MultiBinding TargetTimeline_DeleteSelection;
			Input::MultiBinding TargetTimeline_IncreasePlaybackSpeed;
			Input::MultiBinding TargetTimeline_DecreasePlaybackSpeed;
			Input::MultiBinding TargetTimeline_ToggleMetronome;
			Input::MultiBinding TargetTimeline_ToggleTargetHolds;
			Input::MultiBinding TargetTimeline_PlaceTriangle;
			Input::MultiBinding TargetTimeline_PlaceSquare;
			Input::MultiBinding TargetTimeline_PlaceCross;
			Input::MultiBinding TargetTimeline_PlaceCircle;
			Input::MultiBinding TargetTimeline_PlaceSlideL;
			Input::MultiBinding TargetTimeline_PlaceSlideR;

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
			// bool UnsavedChangesWarningDialog;
			// bool RememberRecentFiles;

			// bool AutoSaveEnabled;
			// TimeSpan AutoSaveInterval;
		} SaveAndLoad;

		struct
		{
			bool ShowButtons;
			bool ShowGrid;
			bool ShowHoldInfo;
			bool ShowBackgroundCheckerboard;
			f32 BackgroundDim;
			Editor::BeatTick PostHitLingerDuration;
			bool DisplayPracticeBackground;
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
		} Playtest;
	};

	// NOTE: Changes are always saved so no need for const protection or manual saves
	inline ComfyStudioAppSettings GlobalAppData = {};

	// NOTE: Global variable for cleaner syntax but const to avoid accidental changes outside an explicit settings window without saving back to disk.
	//		 Modifications should then be done via the Mutable() accessor method.
	inline const ComfyStudioUserSettings GlobalUserData = {};
}
