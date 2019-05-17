#pragma once
#include "../IEditorComponent.h"
#include "../../Audio/AudioEngine.h"
#include "../../Audio/AudioInstance.h"
#include "../../Audio/DummySampleProvider.h"
#include "../../Audio/MemoryAudioStream.h"
#include "../../Audio/Waveform.h"
#include "../../BaseWindow.h"
#include "../../Rendering/Texture.h"
#include "../AudioController.h"
#include "TimelineMap.h"
#include "TargetList.h"
#include <memory>

namespace Editor
{
	enum EditorColor;
	enum VisibilityType { VisibilityType_Visible, VisibilityType_Left, VisibilityType_Right };

	class TargetTimeline : public IEditorComponent, public ICallbackReceiver
	{
	public:
		// Constructors / Destructors:
		// ---------------------------
		TargetTimeline(Application* parent, PvEditor* editor);
		~TargetTimeline();
		// ---------------------------

		// PvEditor Methods:
		// -----------------
		virtual void Initialize() override;
		virtual const char* GetGuiName() override;
		virtual void DrawGui() override;
		virtual void OnLoad() override;
		virtual void OnPlaybackResumed() override;
		virtual void OnPlaybackPaused() override;
		virtual void OnPlaybackStopped() override;
		virtual void OnAudioCallback() override;
		// -----------------

	protected:
		// ----------------------
		struct
		{
			const char* testSongPath = "rom/sound/sngtst.flac";

			std::vector<TimeSpan> buttonSoundTimesList;
			AudioController audioController;

			bool updateWaveform;
			Waveform songWaveform;
		};
		// ----------------------

		// Timeline Regions:
		ImRect timelineRegion;
		ImRect infoColumnHeaderRegion;
		ImRect infoColumnRegion;
		ImRect timelineBaseRegion;
		ImRect tempoMapRegion;
		ImRect timelineHeaderRegion;
		ImRect timelineTargetRegion;
		// -----------------

		// Timeline Zoom:
		// --------------
		const float ZOOM_BASE = 150.0f;
		const float ZOOM_MIN = 1.0f;
		const float ZOOM_MAX = 10.0f;

		bool zoomLevelChanged = false;
		float zoomLevel = 2.0f, lastZoomLevel;
		// --------------

		// Timeline:
		// ---------
		bool updateInput;
		TempoMap tempoMap;
		TimelineMap timelineMap;
		TargetList targets;

		const char* gridDivisionStrings[10] = { "1/1", "1/2", "1/4", "1/8", "1/12", "1/16", "1/24", "1/32", "1/48", "1/64" };
		const int gridDivisions[10] = { 1, 2, 4, 8, 12, 16, 24, 32, 48, 64 };
		int gridDivisionIndex = 0;

		int gridDivision = 16;
		// ---------

		// ----------------------
		float targetYPositions[TargetType_Max];

		// sankaku | shikaku | batsu | maru | slide_l | slide_r | slide_chain_l | slide_chain_r
		static constexpr int buttonIconsTypeCount = 8;
		static constexpr int buttonIconWidth = 52;

		ImRect buttonIconsTextureCoordinates[buttonIconsTypeCount * 2];

		const char* buttonIconsTexturePath = u8"rom/spr/btn_icns.png";
		Texture buttonIconsTexture;

		bool checkHitsoundsInCallback = true;
		struct { bool Down, WasDown; } buttonPlacementKeyStates[6];
		struct { TargetType Type; int Key; } buttonPlacementMapping[6]
		{
			{ TargetType_Sankaku, 'W'},
			{ TargetType_Shikaku, 'A'},
			{ TargetType_Batsu, 'S'},
			{ TargetType_Maru, 'D'},
			{ TargetType_SlideL, 'Q'},
			{ TargetType_SlideR, 'E'},
		};
		// ----------------------

		// ----------------------
		struct
		{
			bool autoScrollCursor = false;
			const float CURSOR_HEAD_WIDTH = 17.0f;
			const float CURSOR_HEAD_HEIGHT = 8.0f;

			// fraction of the timeline width at which the timeline starts scrolling relative to the cursor
			const float autoScrollOffsetFraction = 4.0f;
			TimeSpan cursorTime;
		};

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

		const float timelineVisibleThreshold = 46.0f;
		// --------------------------

		// ----------------------
		ImGuiWindow* baseWindow;
		ImDrawList* baseDrawList;
		// ----------------------

		// ----------------------
		float scrollDelta = 0.0f;
		const float scrollSpeed = 2.0f, scrollSpeedFast = 4.5f;
		// ----------------------

		// ----------------------
		float infoColumnWidth = 46.0f;
		float timelineHeaderHeight = 40.0f - 13.0f;
		float tempoMapHeight = 13.0f;
		// ----------------------

		// ----------------------
		const float ICON_SCALE = 1.0f;
		const float ROW_HEIGHT = 42.0f;
		// ----------------------

		// Child Windows:
		// --------------
		void DrawSyncWindow();
		// --------------

		// ----------------
		void InitializeButtonIcons();
		// ----------------

		// ----------------
		void UpdateRegions();
		void UpdateTimelineMap();
		void UpdateTimelineSize();
		void UpdateOnCallbackSounds();
		void UpdateOnCallbackPlacementSounds();
		// ----------------

		// Timeline Widgets:
		// -----------------
		void DrawTimelineHeaderWidgets();
		// Timeline Column:
		// ----------------
		void DrawTimelineInfoColumnHeader();
		void DrawTimelineInfoColumn();
		// Timeline Base:
		// --------------
		void DrawTimelineBase();
		void DrawTimlineDivisors();
		void DrawWaveform();
		void DrawTimelineTempoMap();
		void DrawTimelineTargets();
		void DrawTimelineCursor();
		void Update();
		void UpdateCursorAutoScroll();
		void UpdateAllInput();
		void UpdateInputPlaybackToggle();
		void UpdateInputCursorClick();
		void UpdateInputTimelineScroll();
		void UpdateInputTargetPlacement();
		// --------------

		// Timeline Actions:
		// -----------------
		void PlaceOrRemoveTarget(TimelineTick tick, TargetType type);
		void SelectNextGridDivision(int direction);
		// -----------------

		// Timeline Control:
		// -----------------
		void CenterCursor();
		bool IsCursorOnScreen();

		inline float GetMaxScrollX() { return ImGui::GetScrollMaxX(); };
		inline float GetScrollX() { return ImGui::GetScrollX(); };
		inline void SetScrollX(float value) { ImGui::SetScrollX(value); };
		// -----------------

		// Conversion Methods:
		// -------------------
		TimelineTick GetGridTick();
		TimelineTick FloorToGrid(TimelineTick tick);
		TimelineTick RoundToGrid(TimelineTick tick);

		float GetTimelinePosition(TimeSpan time);
		float GetTimelinePosition(TimelineTick tick);

		TimelineTick GetTimelineTick(TimeSpan time);
		TimelineTick GetTimelineTick(float position);

		TimeSpan GetTimelineTime(TimelineTick tick);
		TimeSpan GetTimelineTime(float position);

		TimelineTick GetCursorTick();
		TimeSpan GetCursorTime();

		float ScreenToTimelinePosition(float screenPosition);
		float GetCursorTimelinePosition();

		int GetGridDivisionIndex();
		VisibilityType GetTimelineVisibility(float screenX);
		
		ImU32 GetColor(EditorColor color);
		// -------------------

		// -------------------
		float GetButtonTransparency(float screenX);
		int GetButtonIconIndex(const TimelineTarget& target);
		void DrawButtonIcon(ImDrawList* drawList, const TimelineTarget& target, ImVec2 position, float scale, float transparency = 1.0f);
		// -------------------

		// DEBUG STUFF:
		// ------------
		inline void DRAW_DEBUG_REGION(ImRect& rect) { ImGui::AddRectFilled(ImGui::GetForegroundDrawList(), rect, IM_COL32_BLACK * .5f); };
		// ------------
	};
}