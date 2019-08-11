#include "AetTimelineController.h"
#include "AetTimeline.h"

namespace Editor
{
	void MouseSelectionData::Reset() 
	{ 
		RowStartIndex = RowEndIndex = -1; 
		StartX = EndX = -1.0f; 
	};
	
	bool MouseSelectionData::IsSelected() const 
	{ 
		return RowStartIndex >= 0 && RowEndIndex >= 0; 
	};

	AetTimelineController::AetTimelineController()
	{
	}
	
	AetTimelineController::~AetTimelineController()
	{
	}

	void AetTimelineController::UpdateInput(const AetTimeline* timeline)
	{
		updateCursorTime = false;

		if (!ImGui::IsWindowFocused() || ImGui::IsMouseReleased(0))
			isCursorDragging = false;

		vec2 mousePos = ImGui::GetMousePos();
		if (ImGui::IsMouseClicked(0))
		{
			// allow cursor dragging
			if (timeline->GetTimelineHeaderRegion().Contains(mousePos))
			{
				isCursorDragging = true;
			}
			// check for selection start
			else if (timeline->GetTimelineContentRegion().Contains(mousePos))
			{
				selectionData.RowInitialStartIndex = selectionData.RowStartIndex = timeline->GetRowIndexFromScreenY(mousePos.y);
				selectionData.StartX = timeline->GetTimelineFrameAtMouseX();
			}
		}

		if (!ImGui::IsWindowFocused())
			return;

		// dragging has started, don't set RowStartIndex if the mouse is initially hovering over a keyframe
		if (!isCursorDragging && selectionData.RowStartIndex >= 0)
		{
			if (ImGui::IsMouseDown(0))
			{
				selectionData.RowEndIndex = timeline->GetRowIndexFromScreenY(mousePos.y);
				selectionData.EndX = timeline->GetTimelineFrameAtMouseX();

				// clamp selection "height" to be at least one row
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
			else if (ImGui::IsMouseReleased(0))
			{
				selectionData.Reset();
			}
		}

		if (isCursorDragging)
		{
			// sync timeline cursor to mouse cursor

			const TimelineFrame cursorMouseFrame = timeline->GetTimelineFrameAtMouseX();
			TimeSpan previousTime = timeline->GetCursorTime();
			newCursorTime = timeline->GetTimelineTime(cursorMouseFrame);

			if (previousTime == newCursorTime)
				return;

			TimeSpan startTime = timeline->GetTimelineTime(timeline->GetLoopStartFrame());
			TimeSpan endTime = timeline->GetTimelineTime(timeline->GetLoopEndFrame());
			newCursorTime = glm::clamp(newCursorTime.TotalSeconds(), startTime.TotalSeconds(), endTime.TotalSeconds());

			updateCursorTime = true;
		}
	}
}