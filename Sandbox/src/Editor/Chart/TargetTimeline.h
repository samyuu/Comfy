#pragma once
#include "../../BaseWindow.h"
#include "../../Rendering/Texture.h"
#include "TimelineMap.h"
#include "TimelineTick.h"

namespace Editor
{
	class TargetTimeline : public BaseWindow
	{
	public:
		TargetTimeline(Application*);
		~TargetTimeline();

		virtual const char* GetGuiName() override;
		virtual void DrawGui() override;

	protected:

		bool initialized = false;

		const float ZOOM_BASE = 25.0f;

		const float ZOOM_MIN = 1.0f;
		const float ZOOM_MAX = 10.0f;
		float zoomLevel = 1.0f;

		// TODO: need some conversion between TimeStamp, TimelineTick and float TimelinePosition (which gets mulitplied by zoom)
		float GetTimelinePosition(TimelineTick tick);

		TempoMap tempoMap;
		TimelineMap timelineMap;

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

		struct TimelineRow
		{
			TargetType Type;
			//float Height = ROW_HEIGHT;
		};

		TimelineRow timelineRows[TARGET_MAX];

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
		float infoColumnWidth = 46.0f;
		float tempoMapHeaderHeight = 30.0f;

		const float scrollBarHeight = 16.0f;

		const float SCROLL_AMOUNT = 200.0;
		const float ICON_SIZE = 0.35f;

		const float ROW_HEIGHT = 42.0f;
		
		ImU32 GRID_COLOR, GRID_COLOR_ALT, INFO_COLUMN_COLOR, SELECTION_COLOR, TIMELINE_ROW_BG_COLOR;
		
		void Initialize();
		void TimelineHeaderWidgets();
		void TempoMapHeader();
		void TimelineInfoColumn();
		void TimelineChild();
	};
}