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
		BeatTick TimeToTickFixedTempo(TimeSpan time, Tempo tempo) const;

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

	public:
		i32 FindGridDivisionPresetIndex() const;
		void SelectNextPresetGridDivision(i32 direction);

		void AdvanceCursorByGridDivisionTick(i32 direction, bool beatStep = false);
		void AdvanceCursorToNextTarget(i32 direction);

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
		void DrawRangeSelection();

		void DrawTimelineTargets();
		f32 GetTimelineTargetScaleFactor(const TimelineTarget& target, TimeSpan buttonTime) const;

		void DrawTimelineCursor() override;
		void DrawBoxSelection();
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void UpdateKeyboardCtrlInput();
		void UpdateCursorKeyboardInput();

		void UpdateInputSelectionDragging();
		bool CheckIsAnySyncPairPartiallySelected() const;
		bool CheckIsSelectionNotBlocked(BeatTick increment) const;

		void UpdateInputCursorClick();
		void UpdateInputCursorScrubbing();
		void UpdateInputTargetPlacement();
		void UpdateInputContextMenu();
		void UpdateInputBoxSelection();

	private:
		size_t CountSelectedTargets() const;

		void ToggleSelectedTargetsHolds(Chart& chart);

		void SelectAllTargets(Chart& chart);
		void DeselectAllTargets(Chart& chart);
		void SelectEveryNthTarget(Chart& chart, size_t n);
		void ShiftTargetSelection(Chart& chart, i32 direction);
		void RefineTargetSelectionBySingleTargetsOnly(Chart& chart);
		void RefineTargetSelectionBySyncPairsOnly(Chart& chart);
		void SelectAllParticallySelectedSyncPairs(Chart& chart);

		void ClipboardCutSelection();
		void ClipboardCopySelection();
		void ClipboardPasteSelection();

		void FillInRangeSelectionTargets(ButtonType type);
		void PlaceOrRemoveTarget(BeatTick tick, ButtonType type);

		void RemoveAllSelectedTargets(std::optional<size_t> preCalculatedSelectionCount = {});

	private:
		void PlayTargetButtonTypeSound(ButtonType type);

		void PlayCursorButtonSoundsAndAnimation(BeatTick cursorTick);

		void PlaySingleTargetButtonSoundAndAnimation(const TimelineTarget& target);
		void PlaySingleTargetButtonSoundAndAnimation(ButtonType buttonType, BeatTick buttonTick);

		void PlaybackStateChangeSyncButtonSoundCursorTime(TimeSpan newCursorTime);

		f32 GetTimelineSize() const override;
		void OnTimelineBaseScroll() override;

		f32 GetButtonEdgeFadeOpacity(f32 screenX) const;

	private:
		Chart* workingChart = nullptr;

		ChartEditor& chartEditor;
		Undo::UndoManager& undoManager;
		ButtonSoundController& buttonSoundController;

		ClipboardHelper clipboardHelper = {};

		bool isCursorScrubbing = false;

		// NOTE: Store cursor time as BeatTick while paused to avoid floating point precision issues,
		//		 automatically move the cursor while editing the tempo map and to make sure 
		//		 "SetCursorTick(tick); GetCursorTick() == tick;" is always the case
		BeatTick pausedCursorTick = BeatTick::Zero();

	private:
		TimeSpan lastFrameStartOffset = {}, thisFrameStartOffset = {};

		// NOTE: Instead of checking if a button lies between the cursor and last frame cursor time, check if the button will have been pressed in the future
		//		 and then play it back with a negative offset to sample-perfectly sync it to the music.
		//		 The only requirement for this value is that is longer than the duration of any given frame and preferably is as low as possible
		//		 to prevent artifacts when quickly changing the song tempo or offset during playback.
		TimeSpan buttonSoundFutureOffset = TimeSpan::FromSeconds(1.0 / 25.0);

		TimeSpan lastFrameButtonSoundCursorTime = {}, thisFrameButtonSoundCursorTime = {};

		// NOTE: Having this enabled should *discourage* realtime target placement without an accurate tempo map / offset set
		bool buttonSoundOnSuccessfulPlacementOnly = true;

		bool metronomeEnabled = false;
		TimelineMetronome metronome = {};

		bool waveformUpdatePending = true;
		bool waveformDrawIndividualLines = false;

		Audio::Waveform songWaveform;
		Audio::TextureCachedWaveform songTextureCachedWaveform =
		{
			songWaveform,
			std::array { GetColor(EditorColor_WaveformChannel0), GetColor(EditorColor_WaveformChannel1) },
		};

		// NOTE: Because updating the waveform can be quite performance intensive, especially if done every frame like is common when mouse dragging a zoom slider
		const TimeSpan waveformUpdateInterval = COMFY_DEBUG_RELEASE_SWITCH(TimeSpan::FromSeconds(1.0 / 5.0), /*TimeSpan::FromSeconds(1.0 / 30.0)*/TimeSpan::Zero());
		Stopwatch waveformUpdateStopwatch = Stopwatch::StartNew();

	private:
		static constexpr std::array<i32, 10> presetBarGridDivisions = { 4, 8, 12, 16, 24, 32, 48, 64, 96, 192 };
		static constexpr std::array<i32, 6> presetBarChainSlideDivisions = { 12, 16, 24, 32, 48, 64 };

		i32 activeBarGridDivision = 16;
		i32 activeBarChainSlideDivision = 32;

		f32 playbackSpeedStepMin = 0.25f, playbackSpeedStepMax = 1.0f;
		f32 playbackSpeedStep = 0.25f;

	private:
		const f32 iconScale = 1.0f;
		const f32 rowHeight = 36.0f;
		const f32 iconHitboxSize = 34.0f;

		std::vector<vec2> tempSelectedTargetPositionBuffer;

		std::array<f32, EnumCount<ButtonType>()> targetYPositions = {};
		TimelineRenderHelper renderHelper = {};

	private:
		int tempoPopupIndex = -1;

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
