#include "SyncWindow.h"
#include "SortedTempoMap.h"
#include "Editor/Chart/ChartCommands.h"
#include "Timeline/TimelineTick.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	SyncWindow::SyncWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void SyncWindow::OnFirstFrame()
	{
	}

	void SyncWindow::Gui(Chart& chart, TargetTimeline& timeline)
	{
		// ImGuiWindow* syncWindow = Gui::GetCurrentWindow();

		Gui::Text("Adjust Sync:");
		Gui::Separator();

		float startOffset = static_cast<float>(chart.GetStartOffset().TotalMilliseconds());
		if (Gui::InputFloat("Offset##SyncWindow", &startOffset, 1.0f, 10.0f, "%.2f ms"))
			chart.SetStartOffset(TimeSpan::FromMilliseconds(startOffset));

		Gui::Separator();

		if (Gui::InputFloat("Tempo##SyncWindow", &newTempo.BeatsPerMinute, 1.0f, 10.0f, "%.2f BPM"))
			newTempo = glm::clamp(newTempo.BeatsPerMinute, Tempo::MinBPM, Tempo::MaxBPM);

		const float width = Gui::CalcItemWidth();

		if (Gui::Button("Set Tempo Change", vec2(width, 0.0f)))
		{
			TimelineTick cursorTick = timeline.RoundTickToGrid(timeline.GetCursorTick());

			chart.GetTempoMap().SetTempoChange(cursorTick, newTempo);
			timeline.UpdateTimelineMapTimes();
		}

		if (Gui::Button("Remove Tempo Change", vec2(width, 0.0f)))
		{
			TimelineTick cursorTick = timeline.RoundTickToGrid(timeline.GetCursorTick());

			chart.GetTempoMap().RemoveTempoChange(cursorTick);
			timeline.UpdateTimelineMapTimes();
		}
	}
}
