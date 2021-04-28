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

	// TODO: Remove useless controllable interface and make TimelineBaseRegions a member field instead of inheriting..?
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
		inline f32 GetMaxScrollX() const { return maxScroll.x; }
		inline f32 GetScrollX() const { return scroll.x; }

		inline f32 GetMaxScrollY() const { return maxScroll.y; }
		inline f32 GetScrollY() const { return scroll.y; }

		inline const TimelineBaseRegions GetRegions() const { return regions; }

	public:
		void SetZoomCenteredAroundCursor(f32 newZoom);
		void SetZoomCenteredAroundTime(f32 newZoom, TimeSpan timeToCenter);

		inline void SetScrollX(f32 value) { scroll.x = value; }

		inline void SetMaxScrollY(f32 value) { maxScroll.y = value; }
		inline void SetScrollY(f32 value) { scroll.y = value; }

		virtual void CenterCursor(std::optional<f32> widthFactor = {});
		virtual bool IsCursorOnScreen() const;

	protected:
		TimeSpan cursorTime = {};

		TimelineBaseRegions regions = {};

		struct /* TimelineImGuiData */
		{
			ImGuiWindow* baseWindow = nullptr;
			ImDrawList* baseDrawList = nullptr;

			bool updateInput = false;
		};

		struct
		{
			const f32 timelineVisibleThreshold = 96.0f;

			f32 infoColumnWidth = 46.0f;
			f32 infoColumnScrollStep = 42.0f;

			f32 timelineHeaderHeight = 32.0f - 13.0f;
			f32 tempoMapHeight = 13.0f;

			f32 tempoMapFontSize = 14.0f;
			vec2 tempoMapFontOffset = vec2(+1.0f, -0.5f);

			// NOTE: Part of the slider width
			const f32 zoomButtonWidth = 24.0f;
			const f32 zoomSliderWidth = 160.0f;
		};

		static constexpr vec2 timelineScrollbarSize = vec2(14.0f, 16.0f);

		static constexpr f32 zoomBaseFactor = 150.0f;
		const f32 zoomSliderMin = 0.5f;
		const f32 zoomSliderMax = 10.0f;

		// NOTE: Anything outside this range just isn't feasable to handle properly nor does it make sense to ever use
		static constexpr f32 hardZoomLevelMin = 0.01f;
		static constexpr f32 hardZoomLevelMax = 1000.0f;

		struct /* TimelineZoomData */
		{
			bool zoomLevelChanged = false;
			f32 zoomLevel = 2.0f, lastZoomLevel = zoomLevel;
		};

		struct /* TimelineScrollData */
		{
			// NOTE: Percentage of the timeline width at which the timeline starts scrolling to keep the cursor at the same screen position
			f32 autoScrollCursorOffsetPercentage = 0.75f;

			bool autoScrollCursorEnabled = false;
			bool isMouseScrollGrabbing = false;

			f32 scrollDelta = 0.0f;
			f32 scrollSpeed = 2.0f, scrollSpeedFast = 4.5f;

			TimelineScrollbar horizontalScrollbar = { ImGuiAxis_X, timelineScrollbarSize };
			TimelineScrollbar verticalScrollbar = { ImGuiAxis_Y, timelineScrollbarSize };
		};

		static constexpr f32 cursorHeadWidth = 17.0f;
		static constexpr f32 cursorHeadHeight = 8.0f;

	protected:
		void DrawTimelineBase();
		void DrawTimelineZoomSlider();

	protected:
		virtual void PausePlayback() = 0;
		virtual void ResumePlayback() = 0;
		virtual void StopPlayback() = 0;

		virtual void OnDrawTimelineHeaderWidgets() = 0;
		virtual void OnDrawTimelineInfoColumnHeader();
		virtual void OnDrawTimelineInfoColumn();

	protected:
		virtual void OnDrawTimlineTempoMap();
		virtual void OnDrawTimlineRows() = 0;
		virtual void OnDrawTimlineDivisors() = 0;
		virtual void OnDrawTimlineBackground() = 0;
		virtual void DrawTimelineCursor();

	protected:
		virtual void OnDrawTimelineScrollBarRegion() {}

	protected:
		void UpdateInfoColumnInput();

	protected:
		virtual void UpdateTimelineBase();
		virtual void OnUpdate() = 0;
		virtual void OnUpdateInput() = 0;
		virtual void OnDrawTimelineContents() = 0;

		void UpdateTimelineSize();
		virtual void UpdateTimelineRegions();

		virtual void UpdateInputTimelineScroll();
		virtual void UpdateInputPlaybackToggle();

		virtual void OnInfoColumnScroll();
		virtual void OnTimelineBaseScroll();

		virtual void UpdateCursorAutoScroll();

		virtual f32 GetTimelineSize() const = 0;
		virtual f32 GetTimelineHeight() const { return 0.0f; }

	private:
		vec2 scroll = vec2(0.0f, 0.0f);
		vec2 maxScroll = vec2(0.0f, 0.0f);

		// TODO: Implement via SmoothDamp (?)
		// vec2 scrollSmoothness;
		// vec2 scrollTarget = scroll;

		void UpdateAllInput();
	};
}
