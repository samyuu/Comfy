#pragma once
#include "Types.h"
#include "TimelineRenderHelper.h"
#include "TimelineMetronome.h"
#include "Editor/Core/Theme.h"
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Timeline/TimelineBase.h"
#include "Editor/Common/ButtonSoundController.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/ClipboardHelper.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Time/Stopwatch.h"
#include "Undo/Undo.h"
#include <optional>

namespace Comfy::Studio::Editor
{
	class ChartEditor;

	enum class TargetTimelineScalingBehavior : u8
	{
		AutoFit,
		FixedSize,
		Count,
	};

	constexpr std::array<const char*, EnumCount<TargetTimelineScalingBehavior>()> TargetTimelineScalingBehaviorNames =
	{
		"Auto Fit",
		"Fixed Size",
	};

	enum class TargetTimelineCursorScrubbingEdgeAutoScrollThreshold : u8
	{
		Disabled,
		FixedSize,
		Proportional,
		Count,
	};

	constexpr std::array<const char*, EnumCount<TargetTimelineCursorScrubbingEdgeAutoScrollThreshold>()> TargetTimelineCursorScrubbingEdgeAutoScrollThresholdNames =
	{
		"Disabled",
		"Fixed Size",
		"Proportional",
	};

	constexpr f32 TargetTimelineDefaultMouseWheelScrollSpeed = 2.5f;
	constexpr f32 TargetTimelineDefaultMouseWheelScrollSpeedShift = 5.5f;

	constexpr f32 TargetTimelineDefaultCursorScrubbingEdgeAutoScrollProportionalFactorFixedSizePixels = 96.0f;
	constexpr f32 TargetTimelineDefaultCursorScrubbingEdgeAutoScrollProportionalFactor = 0.1f;
	constexpr f32 TargetTimelineMinCursorScrubEdgeAutoScrollWidthFactor = 0.01f;
	constexpr f32 TargetTimelineMaxCursorScrubEdgeAutoScrollWidthFactor = 0.35f;

	constexpr f32 TargetTimelineDefaultPlaybackAutoScrollCursorPositionFactor = 0.35f;
	constexpr f32 TargetTimelineDefaultCursorScrubbingEdgeAutoScrollSmoothScrollSpeedSec = 0.02f;
	constexpr f32 TargetTimelineDefaultCursorScrubbingEdgeAutoScrollSmoothScrollSpeedSecShift = 0.01f;

	constexpr f32 TargetTimelineDefaultIconScale = 1.0f;
	constexpr f32 TargetTimelineMinIconScale = 0.5f;
	constexpr f32 TargetTimelineMaxIconScale = 2.0f;

	constexpr f32 TargetTimelineDefaultRowHeight = 36.0f;
	constexpr f32 TargetTimelineMinRowHeight = (TargetTimelineDefaultRowHeight * TargetTimelineMinIconScale);
	constexpr f32 TargetTimelineMaxRowHeight = (TargetTimelineDefaultRowHeight * TargetTimelineMaxIconScale);

	// NOTE: Subtracted from RowHeight to get final hitbox size
	constexpr f32 TargetTimelineRowHeightHitboxOffset = 2.0f;

	class TargetTimeline final : public TimelineBase
	{
	public:
		TargetTimeline(ChartEditor& parent, Undo::UndoManager& undoManager, ButtonSoundController& buttonSoundController);
		~TargetTimeline() = default;

	public:
		void SetWorkingChart(Chart* chart);

	public:
		void OnSongLoaded();
		void OnPlaybackResumed();
		void OnPlaybackPaused();
		void OnPlaybackStopped();

		void OnEditorSpritesLoaded(const Graphics::SprSet* sprSet);

	public:
		BeatTick GridDivisionTick() const;
		BeatTick ChainSlideDivisionTick() const;

		BeatTick FloorTickToGrid(BeatTick tick) const;
		BeatTick RoundTickToGrid(BeatTick tick) const;

		f32 GetTimelinePosition(TimeSpan time) const override;
		f32 GetTimelinePosition(BeatTick tick) const;

		BeatTick TimeToTick(TimeSpan time) const;

		BeatTick GetBeatTick(f32 position) const;

		TimeSpan TickToTime(BeatTick tick) const;
		TimeSpan GetTimelineTime(f32 position) const override;

		BeatTick GetCursorMouseXTick(bool floorToGrid = true) const;

	public:
		TimeSpan GetCursorTime() const override;
		void SetCursorTime(const TimeSpan newTime);

		BeatTick GetCursorTick() const;
		void SetCursorTick(const BeatTick newTick);

		bool GetIsPlayback() const override;
		void PausePlayback() override;
		void ResumePlayback() override;
		void StopPlayback() override;

		void ResetScrollAndZoom();
		TimelineMetronome& GetMetronome();

		void SetWindowFocusNextFrame(std::optional<i32> frameOffset = {});

	public:
		i32 FindGridDivisionPresetIndex() const;
		void SelectNextPresetGridDivision(i32 direction);
		void SelectNextPlaybackSpeedLevel(i32 direction);

		void AdvanceCursorByGridDivisionTick(i32 direction, bool beatStep = false, i32 distanceFactor = 1);
		void AdvanceCursorToNextTarget(i32 direction);

		void AdvanceCursorAndScrollToStartOrEndOfTimeline(i32 direction);

	private:
		void OnUpdate() override;
		void UpdateOffsetChangeCursorTimeAdjustment();
		void UpdatePlaybackButtonSounds();

	private:
		void OnDrawTimelineHeaderWidgets() override;

	private:
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;

	private:
		void OnDrawTimlineRows() override;
		void OnDrawTimlineDivisors() override;

		void OnDrawTimlineBackground() override;
		void OnDrawTimelineScrollBarRegion() override;
		void DrawOutOfBoundsBackground();
		void DrawCheckUpdateWaveform();
		void DrawTextureCachedWaveform();
		void DrawWaveformIndividualVertexLines();

		void DrawTimelineTempoMap();
		void DrawStartEndMarkers();

		void DrawRangeSelection();

		void DrawTimelineTargets();
		f32 GetTimelineTargetScaleFactor(const TimelineTarget& target, TimeSpan buttonTime) const;

		void DrawTimelineCursor() override;
		void DrawBoxSelection();
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void UpdateKeyboardCtrlInput();
		void UpdateCursorKeyboardInput();

		void UpdateInputSelectionDragging(Undo::UndoManager& undoManager, Chart& chart);
		bool CheckIsAnySyncPairPartiallySelected() const;
		bool CheckIsSelectionNotBlocked(BeatTick increment) const;

		void UpdateInputCursorClick();
		void UpdateInputCursorScrubbing();
		void UpdateInputTargetPlacement();
		void UpdateInputContextMenu();
		void UpdateInputBoxSelection();

	private:
		size_t CountSelectedTargets() const;

		void ToggleSelectedTargetsHolds(Undo::UndoManager& undoManager, Chart& chart);

		void SelectAllTargets(Chart& chart);
		void DeselectAllTargets(Chart& chart);
		void SelectEveryNthTarget(Chart& chart, size_t n);
		void ShiftTargetSelection(Chart& chart, i32 direction);
		void RefineTargetSelectionBySingleTargetsOnly(Chart& chart);
		void RefineTargetSelectionBySyncPairsOnly(Chart& chart);
		void SelectAllParticallySelectedSyncPairs(Chart& chart);

		void ClipboardCutSelection(Undo::UndoManager& undoManager, Chart& chart);
		void ClipboardCopySelection(Undo::UndoManager& undoManager, Chart& chart);
		void ClipboardPasteSelection(Undo::UndoManager& undoManager, Chart& chart);

		BeatTick GetTargetPlacementCursorTickWithAdjustedOffsetSetting() const;

		void FillInRangeSelectionTargets(Undo::UndoManager& undoManager, Chart& chart, ButtonType type);
		void PlaceOrRemoveTarget(Undo::UndoManager& undoManager, Chart& chart, BeatTick tick, ButtonType type);

		void RemoveAllSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, std::optional<size_t> preCalculatedSelectionCount = {});

		void MirrorSelectedTargetTypes(Undo::UndoManager& undoManager, Chart& chart);
		void CompressOrExpandSelectedTargetTimes(Undo::UndoManager& undoManager, Chart& chart, std::array<i32, 2> ratio);

	private:
		void PlayMetronomeToggleSound();
		void PlayTargetButtonTypeSound(ButtonType type);

		void PlayCursorButtonSoundsAndAnimation(BeatTick cursorTick);

		void PlaySingleTargetButtonSoundAndAnimation(const TimelineTarget& target);
		void PlaySingleTargetButtonSoundAndAnimation(ButtonType buttonType, BeatTick buttonTick);

		void PlaybackStateChangeSyncButtonSoundCursorTime(TimeSpan newCursorTime);

		f32 GetTimelineSize() const override;
		void OnTimelineBaseScroll() override;
		void OnTimelineBaseMouseWheelZoom() override;

		f32 GetDerivedClassPlaybackSpeedOverride() const override;
		std::optional<vec2> GetSmoothScrollSpeedSecOverride() const override;

		f32 GetButtonEdgeFadeOpacity(f32 screenX) const;

	private:
		Chart* workingChart = nullptr;

		ChartEditor& chartEditor;
		Undo::UndoManager& undoManager;
		ButtonSoundController& buttonSoundController;

		ClipboardHelper clipboardHelper = {};

		std::optional<i32> guiFrameCountAfterWhichToFocusWindow = {};
		bool isCursorScrubbing = false;
		bool isCursorScrubbingAndPastEdgeAutoScrollThreshold = false;

		// NOTE: Store cursor time as BeatTick while paused to avoid floating point precision issues,
		//		 automatically move the cursor while editing the tempo map and to make sure 
		//		 "SetCursorTick(tick); GetCursorTick() == tick;" is always the case
		BeatTick pausedCursorTick = BeatTick::Zero();

	private:
		TimeSpan lastFrameStartOffset = {}, thisFrameStartOffset = {};

		// NOTE: Instead of checking if a button lies between the cursor and last frame cursor time, check if the button will have been pressed in the future
		//		 and then play it back with a negative offset to sample-perfectly sync it to the music.
		//		 The only requirement for this value is that is longer than the duration of any given frame / audio buffer and preferably is as low as possible
		//		 to prevent artifacts when quickly changing the song tempo or offset during playback.
		TimeSpan buttonSoundFutureOffset = TimeSpan::FromSeconds(1.0 / 25.0);

		// NOTE: To prevent stacking of button sounds that happened "too long" ago, especially when the main window went inactive.
		//		 Should be large enough to never be reached as a normal frametime and longer than the audio render buffer size
		TimeSpan buttonSoundThresholdAtWhichPlayingSoundsMakesNoSense = TimeSpan::FromSeconds(1.0 / 5.0);

		TimeSpan lastFrameButtonSoundCursorTime = {}, thisFrameButtonSoundCursorTime = {};

		// NOTE: Having this enabled should *discourage* realtime target placement without an accurate tempo map / offset set
		bool buttonSoundOnSuccessfulPlacementOnly = true;

		bool metronomeEnabled = false;
		TimelineMetronome metronome = {};

		bool waveformUpdatePending = true;

		Audio::Waveform songWaveform;
		Audio::TextureCachedWaveform songTextureCachedWaveform =
		{
			songWaveform,
			std::array { GetColor(EditorColor_WaveformChannel0), GetColor(EditorColor_WaveformChannel1) },
		};

		// NOTE: Because updating the waveform can be quite performance intensive, especially if done every frame like is common when mouse dragging a zoom slider
		const TimeSpan waveformUpdateInterval = COMFY_DEBUG_RELEASE_SWITCH(TimeSpan::FromSeconds(1.0 / 5.0), /*TimeSpan::FromSeconds(1.0 / 30.0)*/TimeSpan::Zero());
		Stopwatch waveformUpdateStopwatch = Stopwatch::StartNew();

		const TimeSpan waveformFadeInDuration = TimeSpan::FromMilliseconds(240.0f);
		const TimeSpan waveformExpandDuration = TimeSpan::FromMilliseconds(120.0f);
		std::optional<i32> deferredWaveformFadeInStopwatchGuiStartFrame = {};
		Stopwatch waveformFadeInStopwatch = {};

	private:
		i32 activeBarGridDivision = 16;
		i32 activeBarChainSlideDivision = 32;

		f32 playbackSpeedStepMin = 0.25f, playbackSpeedStepMax = 1.0f;
		f32 playbackSpeedStep = 0.25f;

	private:
		f32 iconScale = TargetTimelineDefaultIconScale;
		f32 rowHeight = TargetTimelineDefaultRowHeight;
		f32 iconHitboxSize = (TargetTimelineDefaultRowHeight - TargetTimelineRowHeightHitboxOffset);

		std::vector<vec2> tempSelectedTargetPositionBuffer;

		std::array<f32, EnumCount<ButtonType>()> targetYPositions = {};
		TimelineRenderHelper renderHelper = {};

	private:
		i32 tempoPopupIndex = -1, lastFrameTempoPopupIndex = -1;

		struct
		{
			BeatTick ThisFrameCursorTick, LastFrameCursorTick;
			bool WidgetActiveLastFrame;
		} infoColumnTimeInput = {};

	private:
		struct SelectionDragData
		{
			BeatTick TickOnPress;
			BeatTick LastFrameMouseTick, ThisFrameMouseTick;

			BeatTick TicksMovedSoFar;
			f32 VerticalDistanceMovedSoFar;

			bool IsDragging;
			bool IsHovering;
			bool ChangeType;
		} selectionDrag = {};

		struct BoxSelectionData
		{
			enum class ActionType : u8 { Clean, Add, Remove };

			BeatTick StartTick, EndTick;
			vec2 StartMouse, EndMouse;

			ActionType Action;
			bool IsActive;
			bool IsSufficientlyLarge;
		} boxSelection = {};

		struct RangeSelectionData
		{
			BeatTick StartTick, EndTick;
			bool HasEnd;
			bool IsActive;
		} rangeSelection = {};

	private:

		// NOTE: Delay the animation while placing targets to give them a satisfying "pop" instead of scaling them down continuously
		const TimeSpan buttonAnimationStartTime = TimeSpan::FromMilliseconds(15.0);
		const TimeSpan buttonAnimationDuration = TimeSpan::FromMilliseconds(60.0);

		const f32 buttonAnimationScaleStart = 1.5f, buttonAnimationScaleEnd = 1.0f;

		struct ButtonAnimationData { BeatTick Tick; TimeSpan ElapsedTime; };
		std::array<ButtonAnimationData, EnumCount<ButtonType>()> buttonAnimations = {};
	};
}
