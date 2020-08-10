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
		// NOTE: Negative to visually match the drag direction with that of the waveform timeline position
		constexpr auto offsetDragSpeed = -1.0f;

		auto startOffsetMS = static_cast<f32>(chart.GetStartOffset().TotalMilliseconds());
		if (Gui::DragFloat("Start Offset##SyncWindow", &startOffsetMS, offsetDragSpeed, 0.0f, 0.0f, "%.2f ms"))
			undoManager.Execute<ChangeStartOffset>(chart, TimeSpan::FromMilliseconds(startOffsetMS));

		Gui::Separator();

		const auto cursorTick = timeline.RoundTickToGrid(timeline.GetCursorTick());
		const auto tempoAtCursor = chart.GetTempoMap().FindTempoChangeAtTick(cursorTick).Tempo;

		Gui::TextDisabled("(Cursor %.2f BPM)", tempoAtCursor.BeatsPerMinute);

		if (Gui::DragFloat("Tempo##SyncWindow", &newTempo.BeatsPerMinute, 1.0f, Tempo::MinBPM, Tempo::MaxBPM, "%.2f BPM"))
		{
			newTempo.BeatsPerMinute = std::clamp(newTempo.BeatsPerMinute, Tempo::MinBPM, Tempo::MaxBPM);

			if (chart.GetTempoMap().FindTempoChangeAtTick(cursorTick).Tick != cursorTick)
				undoManager.Execute<AddTempoChange>(chart, cursorTick, newTempo);
			else
				undoManager.Execute<ChangeTempo>(chart, cursorTick, newTempo);
		}

		if (Gui::Button("Remove Tempo Change", vec2(Gui::CalcItemWidth(), 0.0f)))
		{
			undoManager.Execute<RemoveTempoChange>(chart, cursorTick);
		}

		Gui::Separator();

		constexpr auto durationDragSpeed = 10.0f;
		constexpr auto durationMin = static_cast<f32>(TimeSpan::FromMilliseconds(10.0f).TotalMilliseconds());

		auto songDurationMS = static_cast<f32>(chart.GetDuration().TotalMilliseconds());
		if (Gui::DragFloat("Song Duration##SyncWindow", &songDurationMS, durationDragSpeed, durationMin, std::numeric_limits<f32>::max(), "%.2f ms"))
		{
			songDurationMS = std::max(songDurationMS, durationMin);
			undoManager.Execute<ChangeSongDuration>(chart, TimeSpan::FromMilliseconds(songDurationMS));
		}
	}
}
