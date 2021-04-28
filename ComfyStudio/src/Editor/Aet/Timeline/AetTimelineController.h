#pragma once
#include "Editor/Timeline/FrameTimeline.h"

namespace Comfy::Studio::Editor
{
	class AetTimeline;

	// NOTE: Selected left mouse click region
	struct MouseSelectionData
	{
		// NOTE: Index of selected row, so selection always snaps to rows
		i32 RowStartIndex = -1, RowInitialStartIndex;
		i32 RowEndIndex = -1;

		// NOTE: Mouse start/end X in timeline frames to allow for scrolling
		TimelineFrame StartX, EndX;

		void Reset();
		bool IsSelected() const;
	};

	class AetTimelineController
	{
	public:
		AetTimelineController(AetTimeline* timeline);
		~AetTimelineController() = default;

	public:
		// TODO: Pass in list of keyframes, check collision (?)
		void UpdateInput();

		inline bool GetIsCursorDragging() const { return isCursorScrubbing; }
		inline const MouseSelectionData& GetSelectionData() const { return selectionData; }

		inline bool GetUpdateCursorTime() const { return updateCursorTime; }
		inline TimeSpan GetNewCursorTime() const { return newCursorTime; }

	private:
		// NOTE: Set when mouse clicked while inside timelineHeaderRegion, timeline cursor set to mouse cursor
		bool isCursorScrubbing = false;
		MouseSelectionData selectionData;

		bool updateCursorTime = false;
		TimeSpan newCursorTime;

	private:
		AetTimeline* timeline;
	};
}
