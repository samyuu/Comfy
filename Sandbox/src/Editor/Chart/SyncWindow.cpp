#include "SyncWindow.h"
#include "TempoMap.h"
#include "Timeline/TimelineTick.h"
#include "ImGui/imgui_extensions.h"

namespace Editor
{
	SyncWindow::SyncWindow()
	{
	}

	SyncWindow::~SyncWindow()
	{
	}

	void SyncWindow::Initialize()
	{
	}
	
	void SyncWindow::DrawGui(Chart* chart, TargetTimeline* timeline)
	{
		ImGuiWindow* syncWindow = ImGui::GetCurrentWindow();

		ImGui::Text("Adjust Sync:");
		ImGui::Separator();

		float startOffset = static_cast<float>(chart->GetStartOffset().TotalMilliseconds());
		if (ImGui::InputFloat("Offset##SyncWindow", &startOffset, 1.0f, 10.0f, "%.2f ms"))
			chart->SetStartOffset(TimeSpan::FromMilliseconds(startOffset));

		ImGui::Separator();

		if (ImGui::InputFloat("Tempo##SyncWindow", &newTempo.BeatsPerMinute, 1.0f, 10.0f, "%.2f BPM"))
			newTempo = glm::clamp(newTempo.BeatsPerMinute, MIN_BPM, MAX_BPM);

		const float width = ImGui::CalcItemWidth();

		if (ImGui::Button("Set Tempo Change", ImVec2(width, 0)))
		{
			TimelineTick cursorTick = timeline->RoundToGrid(timeline->GetCursorTick());

			chart->GetTempoMap().SetTempoChange(cursorTick, newTempo);
			timeline->UpdateTimelineMap();
		}

		if (ImGui::Button("Remove Tempo Change", ImVec2(width, 0)))
		{
			TimelineTick cursorTick = timeline->RoundToGrid(timeline->GetCursorTick());

			chart->GetTempoMap().RemoveTempoChange(cursorTick);
			timeline->UpdateTimelineMap();
		}
	}
}