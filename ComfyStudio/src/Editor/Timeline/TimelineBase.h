#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Time/TimeSpan.h"
#include "TimelineScrollbar.h"
#include "ImGui/Gui.h"
#include <optional>

namespace Comfy::Studio::Editor
{
	enum class TimelineVisibility : u8
	{
		Visible,
		Left,
		Right,
		Count
	};

	// NOTE: Screenspace regions
	struct TimelineBaseRegions
	{
		// NOTE: The entire timeline without the parent imgui window padding, including scrollbars
		ImRect Base;
		// NOTE: The upper part of the left column
		ImRect InfoColumnHeader;
		// NOTE: The lower part of the left column
		ImRect InfoColumnContent;
		// NOTE: The entire timeline without the left column, excluding scrollbars
		ImRect ContentBase;
		// NOTE: The upper most part of the content base
		ImRect TempoMap;
		// NOTE: The upper middle part of the content base
		ImRect ContentHeader;
		// NOTE: The lower most part of the content base
		ImRect Content;
	};

	class TimelineBase : public NonCopyable
	{
	public:
		TimelineBase() = default;
		virtual ~TimelineBase() = default;

	public:
		virtual bool GetIsPlayback() const = 0;

		virtual f32 GetTimelinePosition(TimeSpan time) const;
		virtual TimeSpan GetTimelineTime(f32 position) const;

		f32 ScreenToTimelinePosition(f32 screenPosition) const;
		f32 GetCursorTimelinePosition() const;

		virtual TimeSpan GetCursorTime() const;

		TimelineVisibility GetTimelineVisibility(f32 screenX) const;
		TimelineVisibility GetTimelineVisibilityForScreenSpace(f32 screenX) const;

		void DrawTimelineGui();

	public:
		inline f32 GetScrollX() const { return scroll.x; }
		inline f32 GetScrollTargetX() const { return scrollTarget.x; }
		inline f32 GetMaxScrollX() const { return scrollMax.x; }

		inline f32 GetScrollY() const { return scroll.y; }
		inline f32 GetScrollTargetY() const { return scrollTarget.y; }
		inline f32 GetMaxScrollY() const { return scrollMax.y; }

		inline void SetScrollTargetX(f32 value) { scrollTarget.x = value; }
		inline void SetScrollTargetY(f32 value) { scrollTarget.y = value; }

		void SetZoomCenteredAroundCursor(f32 newZoom);
		void SetZoomCenteredAroundTime(f32 newZoom, TimeSpan timeToCenter);

		virtual void CenterCursor(std::optional<f32> widthFactor = {});
		virtual bool IsCursorOnScreen() const;

		inline const TimelineBaseRegions& GetRegions() const { return regions; }

	protected:
		void DrawTimelineBase();
		void DrawTimelineZoomSlider();

		virtual void PausePlayback() = 0;
		virtual void ResumePlayback() = 0;
		virtual void StopPlayback() = 0;

		virtual void OnDrawTimelineHeaderWidgets() = 0;
		virtual void OnDrawTimelineInfoColumnHeader();
		virtual void OnDrawTimelineInfoColumn();

		virtual void OnDrawTimlineTempoMap();
		virtual void OnDrawTimlineRows() = 0;
		virtual void OnDrawTimlineDivisors() = 0;
		virtual void OnDrawTimlineBackground() = 0;
		virtual void DrawTimelineCursor();

		virtual void OnDrawTimelineScrollBarRegion() {}

		void UpdateInfoColumnInput();

		virtual void UpdateTimelineBase();
		virtual void OnUpdate() = 0;
		virtual void OnUpdateInput() = 0;
		virtual void OnDrawTimelineContents() = 0;

		virtual void UpdateTimelineRegions();

		virtual void UpdateInputTimelineScroll();
		virtual void UpdateInputPlaybackToggle();

		virtual void OnInfoColumnScroll();
		virtual void OnTimelineBaseScroll();

		virtual void UpdateCursorAutoScroll();

		virtual f32 GetTimelineSize() const = 0;
		virtual f32 GetTimelineHeight() const { return 0.0f; }

	private:
		void UpdateAllInput();

	protected:
		static constexpr f32 visibilityThreshold = 96.0f;
		static constexpr vec2 scrollbarSize = vec2(14.0f, 16.0f);
		static constexpr f32 zoomBaseFactor = 150.0f;

		// NOTE: Anything outside this range just isn't feasable to handle properly nor does it make sense to ever use
		static constexpr f32 hardZoomLevelMin = 0.01f;
		static constexpr f32 hardZoomLevelMax = 1000.0f;
		static constexpr f32 zoomSliderMin = 0.5f;
		static constexpr f32 zoomSliderMax = 10.0f;

		static constexpr f32 cursorHeadWidth = 17.0f;
		static constexpr f32 cursorHeadHeight = 8.0f;

		// NOTE: Part of the slider width
		static constexpr f32 zoomButtonWidth = 24.0f;
		static constexpr f32 zoomSliderWidth = 160.0f;

		TimeSpan cursorTime = {};
		TimelineBaseRegions regions = {};

		ImGuiWindow* baseWindow = nullptr;
		ImDrawList* baseWindowDrawList = nullptr;

		struct
		{
			f32 infoColumnWidth = 46.0f;
			f32 infoColumnScrollStep = 42.0f;
			f32 timelineHeaderHeight = 32.0f - 13.0f;
			f32 tempoMapHeight = 13.0f;
			f32 tempoMapFontSize = 14.0f;
			vec2 tempoMapFontOffset = vec2(+1.0f, -0.5f);
		};

		struct /* TimelineZoomData */
		{
			f32 zoomLevel = 2.0f;
			f32 lastFrameZoomLevel = zoomLevel;
			bool zoomLevelChangedThisFrame = false;
		};

		struct /* TimelineScrollData */
		{
			// NOTE: Percentage of the timeline width at which the timeline starts scrolling to keep the cursor at the same screen position
			f32 autoScrollCursorOffsetPercentage = 0.75f;

			bool autoScrollCursorEnabled = false;
			bool isMouseScrollGrabbing = false;

			f32 mouseScrollSpeed = 2.0f;
			f32 mouseScrollSpeedFast = 4.5f;

			TimelineScrollbar horizontalScrollbar = { ImGuiAxis_X, scrollbarSize };
			TimelineScrollbar verticalScrollbar = { ImGuiAxis_Y, scrollbarSize };
		};

	private:
		vec2 scroll = vec2(0.0f, 0.0f);
		vec2 scrollTarget = scroll;
		vec2 scrollMax = vec2(0.0f, 0.0f);

		vec2 smoothScrollTimeSec = vec2(0.0215f);
		vec2 smoothScrollVelocity = vec2(0.0f);
	};
}
