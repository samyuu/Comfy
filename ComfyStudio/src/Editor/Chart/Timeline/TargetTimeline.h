#pragma once
#include "Types.h"
#include "TimelineButtonIcons.h"
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Timeline/TimelineBase.h"
#include "Editor/Common/ButtonSoundController.h"
#include "Editor/Chart/Chart.h"
#include "Audio/Audio.h"
#include "Input/Input.h"
#include "Time/Stopwatch.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	enum EditorColor;
	class ChartEditor;

	class TargetTimeline final : public TimelineBase
	{
	public:
		TargetTimeline(ChartEditor& parent, Undo::UndoManager& undoManager);
		~TargetTimeline() = default;

	public:
		void OnSongLoaded();
		void OnPlaybackResumed();
		void OnPlaybackPaused();
		void OnPlaybackStopped();

	public:
		TimelineTick GridDivisionTick() const;
		TimelineTick FloorTickToGrid(TimelineTick tick) const;
		TimelineTick RoundTickToGrid(TimelineTick tick) const;

		f32 GetTimelinePosition(TimeSpan time) const override;
		f32 GetTimelinePosition(TimelineTick tick) const;

		TimelineTick TimeToTick(TimeSpan time) const;
		TimelineTick TimeToTickFixedTempo(TimeSpan time, Tempo tempo) const;

		TimelineTick GetTimelineTick(f32 position) const;

		TimeSpan TickToTime(TimelineTick tick) const;
		TimeSpan GetTimelineTime(f32 position) const override;

		TimelineTick GetCursorMouseXTick() const;

	public:
		TimeSpan GetCursorTime() const override;
		void SetCursorTime(const TimeSpan newTime);

		TimelineTick GetCursorTick() const;
		void SetCursorTick(const TimelineTick newTick);

		bool GetIsPlayback() const override;
		void PausePlayback() override;
		void ResumePlayback() override;
		void StopPlayback() override;

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
		void DrawTimelineTargets();

		f32 GetTimelineTargetScaleFactor(const TimelineTarget& target, TimeSpan buttonTime) const;

		void DrawTimelineCursor() override;
		void DrawBoxSelection();
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void UpdateUndoRedoKeyboardInput();
		void UpdateCursorKeyboardInput();
		void UpdateInputCursorClick();
		void UpdateInputTargetPlacement();
		void UpdateInputBoxSelection();

	private:
		void PlaceOrRemoveTarget(TimelineTick tick, ButtonType Type);

	private:
		void PlayCursorButtonSoundsAndAnimation(TimelineTick cursorTick);

		void PlaybackStateChangeSyncButtonSoundCursorTime(TimeSpan newCursorTime);

		f32 GetTimelineSize() const override;
		void OnTimelineBaseScroll() override;

		f32 GetButtonEdgeFadeOpacity(f32 screenX) const;

	private:
		Chart* workingChart = nullptr;

		ChartEditor& chartEditor;
		Undo::UndoManager& undoManager;

		// NOTE: Store cursor time as TimelineTick while paused to avoid floating point precision issues,
		//		 automatically move the cursor while editing the tempo map and to make sure 
		//		 "SetCursorTick(tick); GetCursorTick() == tick;" is always the case
		TimelineTick pausedCursorTick = TimelineTick::Zero();

	private:
		TimeSpan lastFrameStartOffset = {}, thisFrameStartOffset = {};

		// NOTE: Instead of checking if a button lies between the cursor and last frame cursor time, check if the button will have been pressed in the future
		//		 and then play it back with a negative offset to sample-perfectly sync it to the music.
		//		 The only requirement for this value is that is longer than the duration of any given frame and preferably is as low as possible
		//		 to prevent artifacts when quickly changing the song tempo or offset during playback.
		TimeSpan buttonSoundFutureOffset = TimeSpan::FromSeconds(1.0 / 25.0);

		ButtonSoundController buttonSoundController = {};
		TimeSpan lastFrameButtonSoundCursorTime = {}, thisFrameButtonSoundCursorTime = {};

		// NOTE: Having this enabled should *discourage* realtime target placement without an accurate tempo map / offset set
		bool buttonSoundOnSuccessfulPlacementOnly = true;

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
		int activeBarGridDivision = 16;

	private:
		const f32 iconScale = 1.0f;
		const f32 rowHeight = 36.0f;
		const f32 iconHitboxSize = 34.0f;

		std::vector<vec2> tempSelectedTargetPositionBuffer;

		std::array<f32, EnumCount<ButtonType>()> targetYPositions = {};
		std::unique_ptr<TimelineButtonIcons> buttonIcons = nullptr;

		static constexpr struct { ButtonType Type; Input::KeyCode Key; } targetPlacementInputKeyMappings[12]
		{
			{ ButtonType::Triangle, Input::KeyCode_W },
			{ ButtonType::Square, Input::KeyCode_A },
			{ ButtonType::Cross, Input::KeyCode_S },
			{ ButtonType::Circle, Input::KeyCode_D },
			{ ButtonType::SlideL, Input::KeyCode_Q },
			{ ButtonType::SlideR, Input::KeyCode_E },

			{ ButtonType::Triangle, Input::KeyCode_I },
			{ ButtonType::Square, Input::KeyCode_J },
			{ ButtonType::Cross, Input::KeyCode_K },
			{ ButtonType::Circle, Input::KeyCode_L },
			{ ButtonType::SlideL, Input::KeyCode_U },
			{ ButtonType::SlideR, Input::KeyCode_O },
		};

	private:
		int tempoPopupIndex = -1;

	private:
		struct BoxSelectionData
		{
			TimelineTick StartTick, EndTick;
			vec2 StartMouse, EndMouse;

			bool IsActive;
			bool IsSufficientlyLarge;

		} boxSelection = {};

		struct RangeSelectionData
		{
			// TODO:
		} rangeSelection = {};

	private:

		// NOTE: Delay the animation while placing targets to give them a satisfying "pop" instead of scaling them down continuously
		const TimeSpan buttonAnimationStartTime = TimeSpan::FromMilliseconds(15.0);
		const TimeSpan buttonAnimationDuration = TimeSpan::FromMilliseconds(60.0);

		const f32 buttonAnimationScaleStart = 1.5f, buttonAnimationScaleEnd = 1.0f;

		struct ButtonAnimationData { TimelineTick Tick; TimeSpan ElapsedTime; };
		std::array<ButtonAnimationData, EnumCount<ButtonType>()> buttonAnimations = {};
	};
}
