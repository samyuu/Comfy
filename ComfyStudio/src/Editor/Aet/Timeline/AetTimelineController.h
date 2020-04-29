#pragma once
#include "Editor/Timeline/FrameTimeline.h"

namespace Comfy::Editor
{
	class AetTimeline;

	// selected left mouse click region
	struct MouseSelectionData
	{
		// index of selected row, so selection always snaps to rows
		int RowStartIndex = -1, RowInitialStartIndex;
		int RowEndIndex = -1;

		// mouse start/end X in timeline frames to allow for scrolling
		TimelineFrame StartX, EndX;

		void Reset();
		bool IsSelected() const;
	};

	class AetTimelineController
	{
	public:
		AetTimelineController(AetTimeline* timeline);
		~AetTimelineController();

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
