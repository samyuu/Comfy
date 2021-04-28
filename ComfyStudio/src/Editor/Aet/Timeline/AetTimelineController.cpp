#include "AetTimelineController.h"
#include "AetTimeline.h"

namespace Comfy::Studio::Editor
{
	void MouseSelectionData::Reset() 
	{ 
		RowStartIndex = RowEndIndex = -1; 
		StartX = EndX = -1.0f; 
	}
	
	bool MouseSelectionData::IsSelected() const 
	{ 
		return RowStartIndex >= 0 && RowEndIndex >= 0; 
	}

	AetTimelineController::AetTimelineController(AetTimeline* timeline) : timeline(timeline)
	{
	}
	
	void AetTimelineController::UpdateInput()
	{
		updateCursorTime = false;

		if (!Gui::IsWindowFocused() || Gui::IsMouseReleased(0))
			isCursorScrubbing = false;

		vec2 mousePos = Gui::GetMousePos();
		if (Gui::IsMouseClicked(0))
		{
			// NOTE: Allow cursor scrubbing
			if (timeline->GetRegions().ContentHeader.Contains(mousePos))
			{
				isCursorScrubbing = true;
			}
			// NOTE: Check for keyframe collision
			else if (false)
			{
				// TODO: 

				/*
				if (mouseClicked)
				{
					check all keyframes to test for collisions(use circle intersection test)
						then add to selected keyframe vector(maybe add gui temp data variable to keyframe struct itself ie isSelected(? ))

						draw all selected keyframes on a second pass
				}
				*/
			}
			// NOTE: Check for selection start
			else if (timeline->GetRegions().Content.Contains(mousePos))
			{
				selectionData.RowInitialStartIndex = selectionData.RowStartIndex = timeline->GetRowIndexFromScreenY(mousePos.y);
				selectionData.StartX = timeline->GetTimelineFrameAtMouseX();
			}
		}

		if (!Gui::IsWindowFocused())
			return;

		// NOTE: Scrubbing has started, don't set RowStartIndex if the mouse is initially hovering over a keyframe
		if (!isCursorScrubbing && selectionData.RowStartIndex >= 0)
		{
			if (Gui::IsMouseDown(0))
			{
				selectionData.RowEndIndex = timeline->GetRowIndexFromScreenY(mousePos.y);
				selectionData.EndX = timeline->GetTimelineFrameAtMouseX();

				// NOTE: Clamp selection "height" to be at least one row
				if ((selectionData.RowEndIndex == selectionData.RowStartIndex || selectionData.RowEndIndex == selectionData.RowStartIndex + 1) ||
					(selectionData.RowEndIndex > selectionData.RowStartIndex - 1))
				{
					selectionData.RowStartIndex = selectionData.RowInitialStartIndex;
					selectionData.RowEndIndex++;
				}
				else if (selectionData.RowEndIndex == selectionData.RowStartIndex - 1)
				{
					selectionData.RowStartIndex = selectionData.RowInitialStartIndex + 1;
				}
			}
			else if (Gui::IsMouseReleased(0))
			{
				selectionData.Reset();
			}
		}

		if (isCursorScrubbing)
		{
			// NOTE: Sync timeline cursor to mouse cursor

			const TimelineFrame cursorMouseFrame = timeline->GetTimelineFrameAtMouseX();
			const TimeSpan previousTime = timeline->GetCursorTime();
			newCursorTime = timeline->GetTimelineTime(cursorMouseFrame);

			if (previousTime == newCursorTime)
				return;

			const TimeSpan startTime = timeline->GetTimelineTime(timeline->GetLoopStartFrame());
			const TimeSpan endTime = timeline->GetTimelineTime(timeline->GetLoopEndFrame());
			newCursorTime = std::clamp(newCursorTime, startTime, endTime);

			updateCursorTime = true;
		}
	}
}
