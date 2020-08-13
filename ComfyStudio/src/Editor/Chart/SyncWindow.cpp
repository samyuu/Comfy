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
		const auto cursorSitsOnTempoChange = (tempoChangeAtCursor.Tick == cursorTick);

		char rhythmNodeValueBuffer[64];
		sprintf_s(rhythmNodeValueBuffer, "(%.2f BPM - %d/%d)", tempoChangeAtCursor.Tempo.BeatsPerMinute, tempoChangeAtCursor.Signature.Numerator, tempoChangeAtCursor.Signature.Denominator);

		GuiProperty::TreeNode("Chart Rhythm", rhythmNodeValueBuffer, ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			f32 bpm = newTempo.BeatsPerMinute;
			if (GuiProperty::Input("Tempo##SyncWindow", bpm, 1.0f, vec2(Tempo::MinBPM, Tempo::MaxBPM), "%.2f BPM"))
				newTempo.BeatsPerMinute = bpm;

			char signatureFormatBuffer[32];
			sprintf_s(signatureFormatBuffer, "%%d/%d", newSignature.Denominator);

			i32 numerator = newSignature.Numerator;
			if (GuiProperty::Input("Signature##SyncWindow", numerator, 0.1f, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue), signatureFormatBuffer))
				newSignature = TimeSignature(numerator, newSignature.Denominator);

			GuiProperty::PropertyLabelValueFunc("", [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const auto buttonWidth = (Gui::GetContentRegionAvailWidth() - style.ItemSpacing.x) / 2.0f;

				if (cursorSitsOnTempoChange)
				{
					if (Gui::Button("Update##SyncWindow", vec2(buttonWidth, 0.0f)))
						undoManager.Execute<UpdateTempoChange>(chart, TempoChange(cursorTick, newTempo, newSignature));
				}
				else
				{
					if (Gui::Button("Insert##SyncWindow", vec2(buttonWidth, 0.0f)))
						undoManager.Execute<AddTempoChange>(chart, TempoChange(cursorTick, newTempo, newSignature));
				}
				Gui::SameLine();

				// Gui::PushItemDisabledAndTextColorIf(!cursorSitsOnTempoChange);
				if (Gui::Button("Remove##SyncWindow", vec2(buttonWidth, 0.0f)))
				{
					if (cursorSitsOnTempoChange)
						undoManager.Execute<RemoveTempoChange>(chart, cursorTick);
				}
				// Gui::PopItemDisabledAndTextColorIf(!cursorSitsOnTempoChange);

				Gui::PopStyleVar();
				return false;
			});
		});
	}
}
