#pragma once
#include "../../BaseWindow.h"
#include "../../Rendering/Texture.h"
#include "TimelineMap.h"
#include "TimelineTick.h"
#include "Cursor.h"

namespace Editor
{
	enum TargetType
	{
		TARGET_SANKAKU,
		TARGET_SHIKAKU,
		TARGET_BATSU,
		TARGET_MARU,
		TARGET_SLIDE_L,
		TARGET_SLIDE_R,
		TARGET_MAX
	};

	class TargetTimeline : public BaseWindow
	{
	public:
		TargetTimeline(Application*);
		~TargetTimeline();

		virtual const char* GetGuiName() override;
		virtual void DrawGui() override;

	protected:

		// Timeline Regions:
		ImRect timelineRegion;
		ImRect infoColumnHeaderRegion;
		ImRect infoColumnRegion;
		ImRect timelineBaseRegion;
		ImRect tempoMapRegion;
		ImRect timelineHeaderRegion;
		ImRect timelineTargetRegion;
		// -----------------

		bool initialized = false;

		const float ZOOM_BASE = 150.0f;
		const float ZOOM_MIN = 1.0f;
		const float ZOOM_MAX = 10.0f;

		float zoomLevel = 1.0f;
		int gridDivision = 4;

		TempoMap tempoMap;
		TimelineMap timelineMap;

		float targetHeights[TARGET_MAX];
		Texture iconTextures[TARGET_MAX];
		const char* iconPaths[TARGET_MAX] =
		{
			"rom/spr/icon/btn_sankaku.png",
			"rom/spr/icon/btn_shikaku.png",
			"rom/spr/icon/btn_batsu.png",
			"rom/spr/icon/btn_maru.png",
			"rom/spr/icon/btn_slide_l.png",
			"rom/spr/icon/btn_slide_r.png",
		};

		Cursor cursor;

		ImGuiWindow* baseWindow;
		ImDrawList* baseDrawList;
		float scrollDelta = 0.0f;

		float infoColumnWidth = 46.0f;
		float timelineHeaderHeight = 40.0f - 13.0f;
		float tempoMapHeight = 13.0f;
		const float scrollSpeed = 2.0f;

		const float ICON_SIZE = 0.35f;
		const float ROW_HEIGHT = 42.0f;
		
		ImU32 BAR_COLOR, GRID_COLOR, GRID_COLOR_ALT, SELECTION_COLOR;
		ImU32 INFO_COLUMN_COLOR, TEMPO_MAP_BG_COLOR;
		ImU32 TIMELINE_BG_COLOR, TIMELINE_ROW_SEPARATOR_COLOR;
		
		// ----------------
		void Initialize();
		void UpdateRegions();
		// ----------------

		// Timeline Widgets:
		// -----------------
		void TimelineHeaderWidgets();
		// Timeline Column:
		// ----------------
		void TimelineInfoColumnHeader();
		void TimelineInfoColumn();
		// Timeline Base:
		// --------------
		void TimelineBase();
		void TimlineDivisors();
		void TimelineTempoMap();
		void TimelineTargets();
		void TimelineCursor();
		void ScrollControl();
		// --------------

		// Conversion Methods:
		// -------------------
		float GetTimelinePosition(TimeSpan time);
		float GetTimelinePosition(TimelineTick tick);
		TimelineTick GetTimelineTick(float position);
		TimeSpan GetTimelineTime(TimelineTick tick);
		TimeSpan GetTimelineTime(float position);
		float ScreenToTimelinePosition(float screenPosition);
		// -------------------

		// DEBUG STUFF
		// -----------
		inline void DRAW_DEBUG_REGION(ImRect& rect) { ImGui::AddRectFilled(ImGui::GetForegroundDrawList(), rect, IM_COL32_BLACK * .5f); }
		// -----------
	};
}