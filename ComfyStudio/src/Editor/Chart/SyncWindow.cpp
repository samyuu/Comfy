#include "SyncWindow.h"
#include "SortedTempoMap.h"
#include "Editor/Chart/ChartCommands.h"
#include "Timeline/TimelineTick.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"

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
		GuiPropertyRAII::PropertyValueColumns columns;

		GuiProperty::TreeNode("Song Sync", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			// NOTE: Negative to visually match the drag direction with that of the waveform timeline position
			constexpr auto offsetDragSpeed = -1.0f;

			auto startOffsetMS = static_cast<f32>(chart.GetStartOffset().TotalMilliseconds());
			if (GuiProperty::Input("Start Offset##SyncWindow", startOffsetMS, offsetDragSpeed, {}, "%.2f ms"))
				undoManager.Execute<ChangeStartOffset>(chart, TimeSpan::FromMilliseconds(startOffsetMS));

			GuiProperty::PropertyFuncValueFunc([&]
			{
				Gui::TextDisabled("Move first Beat");
				return false;
			}, [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const auto buttonWidth = (Gui::GetContentRegionAvailWidth() - style.ItemSpacing.x) / 4.0f;

				auto beatOffsetButton = [&](const char* label, const f64 factor)
				{
					if (Gui::Button(label, vec2(buttonWidth, 0.0f)))
					{
						const auto firstTempo = chart.GetTempoMap().FindTempoChangeAtTick(TimelineTick::Zero()).Tempo;
						const auto beatDuration = TimeSpan::FromSeconds(60.0 / firstTempo.BeatsPerMinute);

						undoManager.Execute<ChangeStartOffset>(chart, TimeSpan::FromMilliseconds(startOffsetMS) + (beatDuration * factor));
					}
				};

				beatOffsetButton("+1.0", +1.0);
				Gui::SameLine();
				beatOffsetButton("+0.5", +0.5);
				Gui::SameLine();
				beatOffsetButton("-0.5", -0.5);
				Gui::SameLine();
				beatOffsetButton("-1.0", -1.0);

				Gui::PopStyleVar();
				return false;
			});

			constexpr auto durationDragSpeed = 10.0f;
			constexpr auto durationMin = static_cast<f32>(TimeSpan::FromMilliseconds(10.0f).TotalMilliseconds());
			constexpr auto durationMax = std::numeric_limits<f32>::max();

			auto songDurationMS = static_cast<f32>(chart.GetDuration().TotalMilliseconds());
			if (GuiProperty::Input("Duration##SyncWindow", songDurationMS, durationDragSpeed, vec2(durationMin, durationMax), "%.2f ms"))
			{
				songDurationMS = std::max(songDurationMS, durationMin);
				undoManager.Execute<ChangeSongDuration>(chart, TimeSpan::FromMilliseconds(songDurationMS));
			}
		});

		const auto cursorTick = timeline.RoundTickToGrid(timeline.GetCursorTick());
		const auto tempoChangeAtCursor = chart.GetTempoMap().FindTempoChangeAtTick(cursorTick);

		char rhythmNodeValueBuffer[64];
		sprintf_s(rhythmNodeValueBuffer, "(%.2f BPM)", tempoChangeAtCursor.Tempo.BeatsPerMinute);

		GuiProperty::TreeNode("Chart Rhythm", rhythmNodeValueBuffer, ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			if (GuiProperty::Input("Tempo##SyncWindow", newTempo.BeatsPerMinute, 1.0f, vec2(Tempo::MinBPM, Tempo::MaxBPM), "%.2f BPM"))
				newTempo.BeatsPerMinute = std::clamp(newTempo.BeatsPerMinute, Tempo::MinBPM, Tempo::MaxBPM);

			GuiProperty::PropertyLabelValueFunc("", [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const auto buttonWidth = (Gui::GetContentRegionAvailWidth() - style.ItemSpacing.x) / 2.0f;

				if (Gui::Button("Insert##SyncWindow", vec2(buttonWidth, 0.0f)))
				{
					if (tempoChangeAtCursor.Tick != cursorTick)
						undoManager.Execute<AddTempoChange>(chart, cursorTick, newTempo);
					else
						undoManager.Execute<ChangeTempo>(chart, cursorTick, newTempo);
				}
				Gui::SameLine();
				if (Gui::Button("Remove##SyncWindow", vec2(buttonWidth, 0.0f)))
				{
					if (tempoChangeAtCursor.Tick == cursorTick)
						undoManager.Execute<RemoveTempoChange>(chart, cursorTick);
				}

				Gui::PopStyleVar();
				return false;
			});
		});
	}
}
