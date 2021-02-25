#pragma once
#include "Time/TimeSpan.h"
#include "TimelineBaseRegions.h"
#include "ITimelinePlaybackControllable.h"
#include "TimelineScrollbar.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	enum class TimelineVisibility
	{
		Visible,
		Left,
		Right
	};

	class TimelineBase : public TimelineBaseRegions, public ITimelinePlaybackControllable
	{
	public:
		TimelineBase() = default;
		virtual ~TimelineBase() = default;

	public:
		virtual float GetTimelinePosition(TimeSpan time) const;
		virtual TimeSpan GetTimelineTime(float position) const;

		float ScreenToTimelinePosition(float screenPosition) const;
		float GetCursorTimelinePosition() const;

		virtual TimeSpan GetCursorTime() const;

		TimelineVisibility GetTimelineVisibility(float screenX) const;
		TimelineVisibility GetTimelineVisibilityForScreenSpace(float screenX) const;

		void DrawTimelineGui();

	public:
		inline float GetMaxScrollX() const { return maxScroll.x; }
		inline float GetScrollX() const { return scroll.x; }

		inline float GetMaxScrollY() const { return maxScroll.y; }
		inline float GetScrollY() const { return scroll.y; }

	public:
		void SetZoomCenteredAroundCursor(float newZoom);
		void SetZoomCenteredAroundTime(float newZoom, TimeSpan timeToCenter);

		inline void SetScrollX(float value) { scroll.x = value; }

		inline void SetMaxScrollY(float value) { maxScroll.y = value; }
		inline void SetScrollY(float value) { scroll.y = value; }

		virtual void CenterCursor();
		virtual bool IsCursorOnScreen() const;

	protected:
		TimeSpan cursorTime = {};

		struct /* TimelineImGuiData */
		{
			ImGuiWindow* baseWindow = nullptr;
			ImDrawList* baseDrawList = nullptr;

			bool updateInput = false;
		};

		struct
		{
			const float timelineVisibleThreshold = 96.0f;

			float infoColumnWidth = 46.0f;
			float infoColumnScrollStep = 42.0f;

			float timelineHeaderHeight = 32.0f - 13.0f;
			float tempoMapHeight = 13.0f;

			float tempoMapFontSize = 14.0f;
			vec2 tempoMapFontOffset = vec2(+1.0f, -0.5f);

			// NOTE: Part of the slider width
			const float zoomButtonWidth = 24.0f;
			const float zoomSliderWidth = 160.0f;
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
			float zoomLevel = 2.0f, lastZoomLevel = zoomLevel;
		};

		struct /* TimelineScrollData */
		{
			// NOTE: Percentage of the timeline width at which the timeline starts scrolling to keep the cursor at the same screen position
			float autoScrollCursorOffsetPercentage = 0.75f;

			bool autoScrollCursorEnabled = false;
			bool isMouseScrollGrabbing = false;

			float scrollDelta = 0.0f;
			float scrollSpeed = 2.0f, scrollSpeedFast = 4.5f;

			TimelineScrollbar horizontalScrollbar = { ImGuiAxis_X, timelineScrollbarSize };
			TimelineScrollbar verticalScrollbar = { ImGuiAxis_Y, timelineScrollbarSize };
		};

		static constexpr float cursorHeadWidth = 17.0f;
		static constexpr float cursorHeadHeight = 8.0f;

	protected:
		void DrawTimelineBase();
		void DrawTimelineZoomSlider();

	protected:
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
		virtual void UpdateTimelineRegions() override;

		virtual void UpdateInputTimelineScroll();
		virtual void UpdateInputPlaybackToggle();

		virtual void OnInfoColumnScroll();
		virtual void OnTimelineBaseScroll();

		virtual void UpdateCursorAutoScroll();

		virtual float GetTimelineSize() const = 0;
		virtual float GetTimelineHeight() const { return 0.0f; }

	private:
		vec2 scroll = vec2(0.0f, 0.0f);
		vec2 maxScroll = vec2(0.0f, 0.0f);

		// TODO: 
		// vec2 scrollSmoothness;
		// vec2 scrollTarget = scroll;

		void UpdateAllInput();
	};
}
