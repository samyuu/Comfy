#pragma once
#include "Types.h"
#include "Time/TimeSpan.h"
#include "Time/Stopwatch.h"
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

	constexpr f32 TimelineDefaultMouseWheelScrollSpeed = 2.0f;
	constexpr f32 TimelineDefaultMouseWheelScrollSpeedShift = 4.5f;
	constexpr f32 TimelineDefaultPlaybackAutoScrollCursorPositionFactor = 0.75f;

	constexpr vec2 TimelineDefaultSmoothScrollSpeedSec = vec2(0.0215f);
	constexpr vec2 TimelineMinSmoothScrollSpeedSec = vec2(0.01f);
	constexpr vec2 TimelineMaxSmoothScrollSpeedSec = vec2(0.15f);

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

		inline void SetScrollTargetX(f32 value) { scrollTarget.x = value; InvalidateAutoScrollLock(); }
		inline void SetScrollTargetY(f32 value) { scrollTarget.y = value; }

		// NOTE: Should be called every time the cursor screen position might have been changed
		void InvalidateAutoScrollLock();
		bool IsCursorAutoScrollLocked() const;
		bool WasCursorAutoScrollLockedAtLeastOnceSincePlaybackStart() const;

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

		virtual void OnUpdate() = 0;
		virtual void OnUpdateInput() = 0;
		virtual void OnDrawTimelineContents() = 0;

		virtual void UpdateTimelineRegions();

		virtual void UpdateInputTimelineScroll();
		virtual void UpdateInputPlaybackToggle();

		virtual void OnInfoColumnScroll();
		virtual void OnTimelineBaseScroll();
		virtual void OnTimelineBaseMouseWheelZoom();

		f32 UpdateCursorAutoScrollX();

		virtual f32 GetTimelineSize() const = 0;
		virtual f32 GetTimelineHeight() const { return 0.0f; }

		// HACK: Too much inheritance and it's starting to get out of hand... but the speed is needed for auto scroll locking
		virtual f32 GetDerivedClassPlaybackSpeedOverride() const { return 1.0f; }
		virtual std::optional<vec2> GetSmoothScrollSpeedSecOverride() const { return {}; }

	private:
		void UpdateAllInput();

	protected:
		static constexpr f32 visibilityThreshold = 96.0f;
		static constexpr vec2 scrollbarSize = vec2(14.0f, 16.0f);
		static constexpr f32 zoomBaseFactor = 150.0f;

		// NOTE: Anything outside this range just isn't feasable to handle properly nor does it make sense to ever use
		static constexpr f32 hardZoomLevelMin = 0.01f;
		static constexpr f32 hardZoomLevelMax = 15.0f;
		static constexpr f32 zoomSliderMin = 0.5f;
		static constexpr f32 zoomSliderMax = 5.0f;

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
			f32 playbackAutoScrollCursorPositionFactor = TimelineDefaultPlaybackAutoScrollCursorPositionFactor;

			// NOTE: To ensure the cursor stays on the exact same pixel position during auto scrolling without "jiggling" around
			bool enablePlaybackAutoScrollLocking = false;
			bool lockCursorToAutoScrollPosition = false;
			bool wasCursorAutoScrollLockedAtLeastOnceSincePlaybackStart = false;
			ImRect lastFrameContentRectForAutoScrollInvalidation = {};
			f32 lastFramePlaybackSpeedForAutoScrollInvalidation = {};
			// NOTE: To avoid any sudden cursor "skips" when locking onto the auto scroll position
			Stopwatch lastAutoScrollLockStopwatch = {};
			TimeSpan realAndAutoScrollLockedCursorPositionTransitionTime = TimeSpan::FromMilliseconds(90.0/*60.0*/);

			// DEBUG: To confirm that auto scroll (together with smooth scroll) is working as intended
			bool debugVisualizeAutoScrollCursorPositionByDrawingAdditionalCursorLine = false;
			// DEBUG: To confirm that auto scroll cursor snapping is working as intended
			bool debugVisualizeLockedAutoScrollCursorStateByUsingDifferentColors = false;

			bool isMouseScrollGrabbing = false;

			f32 mouseScrollSpeed = TimelineDefaultMouseWheelScrollSpeed;
			f32 mouseScrollSpeedShift = TimelineDefaultMouseWheelScrollSpeedShift;

			TimelineScrollbar horizontalScrollbar = { ImGuiAxis_X, scrollbarSize };
			TimelineScrollbar verticalScrollbar = { ImGuiAxis_Y, scrollbarSize };
		};

		// NOTE: Time in seconds it takes to reach the current scroll target. Set to <= 0.0f to disable smooth scrolling
		vec2 smoothScrollSpeedSec = TimelineDefaultSmoothScrollSpeedSec;

	private:
		vec2 smoothScrollVelocity = vec2(0.0f);

		vec2 scroll = vec2(0.0f, 0.0f);
		vec2 scrollTarget = scroll;
		vec2 scrollMax = vec2(0.0f, 0.0f);
	};
}
