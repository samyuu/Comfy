#pragma once
#include "Editor/IEditorComponent.h"
#include "Editor/Timeline/TimelineBase.h"
#include "Editor/AudioController.h"
#include "Editor/Chart/Chart.h"
#include "Audio/Waveform.h"
#include "Graphics/Texture.h"
#include "FileSystem/Format/SprSet.h"
#include "Input/KeyCode.h"

namespace Editor
{
	enum EditorColor;
	class ChartEditor;

	class TargetTimeline : public TimelineBase, public ICallbackReceiver
	{
	public:
		TargetTimeline(ChartEditor* parentChartEditor);
		~TargetTimeline();

		void Initialize();

		void OnSongLoaded();
		void OnPlaybackResumed();
		void OnPlaybackPaused();
		void OnPlaybackStopped();

		virtual void OnAudioCallback() override;

		// Conversion Methods:
		// -------------------
		TimelineTick GetGridTick() const;
		TimelineTick FloorToGrid(TimelineTick tick) const;
		TimelineTick RoundToGrid(TimelineTick tick) const;

		float GetTimelinePosition(TimeSpan time) const override;
		float GetTimelinePosition(TimelineTick tick) const;

		TimelineTick GetTimelineTick(TimeSpan time) const;
		TimelineTick GetTimelineTick(float position) const;

		TimeSpan GetTimelineTime(TimelineTick tick) const;
		TimeSpan GetTimelineTime(float position) const override;

		TimelineTick GetCursorTick() const;
		TimelineTick GetCursorMouseXTick() const;

		int GetGridDivisionIndex() const;
		// -------------------

		void UpdateTimelineMap();
		// -------------------

	protected:
		Chart* chart;
		ChartEditor* chartEditor;

		// ----------------------
		std::vector<TimeSpan> buttonSoundTimesList;
		AudioController audioController;

		bool updateWaveform;
		Waveform songWaveform;
		// ----------------------

		// Timeline:
		// ---------
		const std::array<const char*, 10> gridDivisionStrings = { "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
		const std::array<int, 10> gridDivisions = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64 };

		int gridDivisionIndex = 0;
		int gridDivision = 16;
		// ----------------------

		std::array<float, TargetType_Max> targetYPositions;

		// sankaku | shikaku | batsu | maru | slide_l | slide_r | slide_chain_l | slide_chain_r
		static constexpr int buttonIconsTypeCount = 8;
		static constexpr int buttonIconWidth = 52;

		std::array<ImRect, buttonIconsTypeCount * 2> buttonIconsTextureCoordinates;
		FileSystem::SprSet sprSet;
		Texture2D* buttonIconsTexture;

		bool checkHitsoundsInCallback = false;
		struct { bool Down, WasDown; } buttonPlacementKeyStates[12];
		struct { TargetType Type; KeyCode Key; } buttonPlacementMapping[12]
		{
			{ TargetType_Sankaku, KeyCode_W },
			{ TargetType_Shikaku, KeyCode_A },
			{ TargetType_Batsu, KeyCode_S },
			{ TargetType_Maru, KeyCode_D },
			{ TargetType_SlideL, KeyCode_Q },
			{ TargetType_SlideR, KeyCode_E },

			{ TargetType_Sankaku, KeyCode_I },
			{ TargetType_Shikaku, KeyCode_J },
			{ TargetType_Batsu, KeyCode_K },
			{ TargetType_Maru, KeyCode_L },
			{ TargetType_SlideL, KeyCode_U },
			{ TargetType_SlideR, KeyCode_O },
		};
		// ----------------------

		// --------------
		char timeInputBuffer[32] = "00:00.000";
		// --------------
		bool timeSelectionActive = false;
		TimelineTick timeSelectionStart, timeSelectionEnd;
		// --------------

		// Timeline Button Animation:
		// --------------------------
		const TimeSpan buttonAnimationStartTime = TimeSpan::FromMilliseconds(15.0);
		const TimeSpan buttonAnimationDuration = TimeSpan::FromMilliseconds(60.0);
		const float buttonAnimationScale = 1.5f;
		struct
		{
			TimelineTick Tick;
			TimeSpan ElapsedTime;
		} buttonAnimations[TargetType_Max];
		// --------------------------

		// ----------------------
		const float ICON_SCALE = 1.0f;
		const float ROW_HEIGHT = 36;
		// ----------------------

	protected:
		void InitializeButtonIcons();

		// ----------------
		void OnUpdate() override;
		void UpdateOnCallbackSounds();
		void UpdateOnCallbackPlacementSounds();
		// ----------------

		// Timeline Widgets:
		// -----------------
		void OnDrawTimelineHeaderWidgets() override;
		// Timeline Column:
		// ----------------
		void OnDrawTimelineInfoColumnHeader() override;
		void OnDrawTimelineInfoColumn() override;
		// Timeline Base:
		// --------------
		void OnDrawTimlineRows() override;
		void OnDrawTimlineDivisors() override;
		void OnDrawTimlineBackground() override;
		void DrawWaveform();
		void DrawTimelineTempoMap();
		void DrawTimelineTargets();
		void DrawTimelineCursor() override;
		void DrawTimeSelection();
		void OnUpdateInput() override;
		void OnDrawTimelineContents() override;
		void UpdateInputCursorClick();
		void UpdateInputTargetPlacement();
		// --------------

		// Timeline Actions:
		// -----------------
		void PlaceOrRemoveTarget(TimelineTick tick, TargetType type);
		void SelectNextGridDivision(int direction);
		// -----------------

		// Timeline Control:
		// -----------------
		virtual TimeSpan GetCursorTime() const override;
		virtual bool GetIsPlayback() const override;
		virtual void PausePlayback() override;
		virtual void ResumePlayback() override;
		virtual void StopPlayback() override;

		virtual float GetTimelineSize() const override;
		void OnTimelineBaseScroll() override;
		// -----------------

		// -------------------
		float GetButtonTransparency(float screenX) const;
		int GetButtonIconIndex(const TimelineTarget& target) const;
		void DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, vec2 position, float scale, float transparency = 1.0f);
		// -------------------
	};
}