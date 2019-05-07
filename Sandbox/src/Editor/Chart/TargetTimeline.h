#pragma once
#include "../../BaseWindow.h"
#include "../../Rendering/Texture.h"
#include "TimelineMap.h"
#include "TimelineTick.h"

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

		bool initialized = false;

		const float ZOOM_BASE = 150.0f;

		const float ZOOM_MIN = 1.0f;
		const float ZOOM_MAX = 10.0f;
		float zoomLevel = 1.0f;
		int gridDivision = 4;

		// TODO: need some conversion between TimeStamp, TimelineTick and float TimelinePosition (which gets mulitplied by zoom)
		float GetTimelinePosition(TimeSpan time);
		float GetTimelinePosition(TimelineTick tick);

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

		ImGuiWindow* parentWindow;
		float scrollDelta = 0.0f;

		float infoColumnWidth = 46.0f;
		float tempoMapHeaderHeight = 40.0f;
		float tempoMapBarHeight = 13.0f;
		const float scrollSpeed = 2.0f;

		const float ICON_SIZE = 0.35f;
		const float ROW_HEIGHT = 42.0f;
		
		ImU32 BAR_COLOR, GRID_COLOR, GRID_COLOR_ALT, SELECTION_COLOR;
		ImU32 INFO_COLUMN_COLOR, TEMPO_MAP_BAR_COLOR;
		ImU32 TIMELINE_BG_COLOR, TIMELINE_ROW_SEPARATOR_COLOR;
		
		void Initialize();
		void TimelineHeaderWidgets();
		void TempoMapHeader();
		void TimelineInfoColumn();
		void TimelineChild();
	};
}