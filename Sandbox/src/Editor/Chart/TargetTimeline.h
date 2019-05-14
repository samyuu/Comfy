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
		TempoMap tempoMap;
		TimelineMap timelineMap;
		TargetList targets;
		int gridDivision = 16;
		// ---------

		// ----------------------
		float targetYPositions[TargetType_Max];
		Texture iconTextures[TargetType_Max];
		const char* iconPaths[TargetType_Max] =
		{
			"rom/spr/icon/btn_sankaku.png",
			"rom/spr/icon/btn_shikaku.png",
			"rom/spr/icon/btn_batsu.png",
			"rom/spr/icon/btn_maru.png",
			"rom/spr/icon/btn_slide_l.png",
			"rom/spr/icon/btn_slide_r.png",
		};
		// ----------------------

		// ----------------------
		struct
		{
			const float CURSOR_HEAD_WIDTH = 17.0f;
			const float CURSOR_HEAD_HEIGHT = 8.0f;

			// fraction of the timeline width at which the timeline starts scrolling relative to the cursor
			const float autoScrollOffsetFraction = 4.0f;
		};
		// ----------------------

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
		const float ICON_SIZE = 0.35f;
		const float ROW_HEIGHT = 42.0f;
		// ----------------------

		// Timeline Colors:
		// ----------------
		ImU32 BAR_COLOR, GRID_COLOR, GRID_COLOR_ALT, SELECTION_COLOR;
		ImU32 INFO_COLUMN_COLOR, TEMPO_MAP_BG_COLOR;
		ImU32 TIMELINE_BG_COLOR, TIMELINE_ROW_SEPARATOR_COLOR;
		ImU32 CURSOR_COLOR = ImColor(0.71f, 0.54f, 0.15f);
		// ----------------

		// Child Windows:
		// --------------
		void DrawSyncWindow();
		// --------------

		// ----------------
		void UpdateRegions();
		void UpdateTimelineMap();
		void UpdateTimelineSize();
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
		void DrawTimelineTempoMap();
		void DrawTimelineTargets();
		void DrawTimelineCursor();
		void Update();
		void UpdateAllInput();
		void UpdateInputCursorClick();
		void UpdateInputTimelineScroll();
		// --------------

		// Timeline Control:
		// -----------------
		void CenterCursor();
		inline float GetScrollX() { return ImGui::GetScrollX(); };
		inline void SetScrollX(float value) { ImGui::SetScrollX(value); };
		// -----------------

		// Conversion Methods:
		// -------------------
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
		// -------------------

		// DEBUG STUFF:
		// ------------
		inline void DRAW_DEBUG_REGION(ImRect& rect) { ImGui::AddRectFilled(ImGui::GetForegroundDrawList(), rect, IM_COL32_BLACK * .5f); };
		// ------------
	};
}