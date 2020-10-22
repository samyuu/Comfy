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

	void SyncWindow::Gui(Chart& chart, TargetTimeline& timeline)
	{
		GuiPropertyRAII::PropertyValueColumns columns;

		GuiProperty::TreeNode("Song Sync", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			// NOTE: Negative to visually match the drag direction with that of the waveform timeline position
			constexpr auto offsetDragSpeed = -1.0f;

			auto startOffsetMS = static_cast<f32>(chart.StartOffset.TotalMilliseconds());
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
						const auto firstTempo = chart.TempoMap.FindTempoChangeAtTick(TimelineTick::Zero()).Tempo;
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

			// TODO: Option (maybe via context menu (?)) to set duration equal to song source duration + offset
			constexpr auto durationDragSpeed = 10.0f;
			constexpr auto durationMinMS = static_cast<f32>(TimeSpan::FromSeconds(1.0).TotalMilliseconds());
			constexpr auto durationMaxMS = std::numeric_limits<f32>::max();

			auto songDurationMS = static_cast<f32>(chart.DurationOrDefault().TotalMilliseconds());
			if (GuiProperty::Input("Duration##SyncWindow", songDurationMS, durationDragSpeed, vec2(durationMinMS, durationMaxMS), "%.2f ms"))
			{
				songDurationMS = std::max(songDurationMS, durationMinMS);
				undoManager.Execute<ChangeSongDuration>(chart, TimeSpan::FromMilliseconds(songDurationMS));
			}
		});

		lastFrameCursorTick = thisFrameCursorTick;
		thisFrameCursorTick = timeline.RoundTickToGrid(timeline.GetCursorTick());

		const auto tempoChangeAtCursor = chart.TempoMap.FindTempoChangeAtTick(thisFrameCursorTick);
		const auto cursorSitsOnTempoChange = (tempoChangeAtCursor.Tick == thisFrameCursorTick);

		auto tempoComparison = [](const auto& a, const auto& b) { return (a.Tempo.BeatsPerMinute < b.Tempo.BeatsPerMinute); };
		const auto minBPM = (chart.TempoMap.TempoChangeCount() < 1) ? 0.0f : std::min_element(chart.TempoMap.begin(), chart.TempoMap.end(), tempoComparison)->Tempo.BeatsPerMinute;
		const auto maxBPM = (chart.TempoMap.TempoChangeCount() < 1) ? 0.0f : std::max_element(chart.TempoMap.begin(), chart.TempoMap.end(), tempoComparison)->Tempo.BeatsPerMinute;

		char rhythmNodeValueBuffer[64];
		const auto rhythmNodeValueView = std::string_view(rhythmNodeValueBuffer, (minBPM == maxBPM) ?
			sprintf_s(rhythmNodeValueBuffer, "(%.2f BPM)", minBPM) :
			sprintf_s(rhythmNodeValueBuffer, "(%.2f - %.2f BPM)", minBPM, maxBPM));

		GuiProperty::TreeNode("Chart Rhythm", rhythmNodeValueView, ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			auto executeAddOrUpdate = [&]()
			{
				// HACK: Moving the cursor on the timeline (= losing focus on the same frame) while having a textbox focused
				//		 would otherwise incorrectly insert an additional tempo change at the new cursor location
				if (thisFrameCursorTick != lastFrameCursorTick)
					return;

				if (cursorSitsOnTempoChange)
					undoManager.Execute<UpdateTempoChange>(chart, TempoChange(thisFrameCursorTick, newTempo, newSignature));
				else
					undoManager.Execute<AddTempoChange>(chart, TempoChange(thisFrameCursorTick, newTempo, newSignature));
			};

			newTempo = tempoChangeAtCursor.Tempo;
			newSignature = tempoChangeAtCursor.Signature;

			if (GuiProperty::Input("Tempo##SyncWindow", newTempo.BeatsPerMinute, 1.0f, vec2(Tempo::MinBPM, Tempo::MaxBPM), "%.2f BPM"))
				executeAddOrUpdate();

			char signatureFormatBuffer[32];
			sprintf_s(signatureFormatBuffer, "%%d/%d", newSignature.Denominator);

			i32 numerator = newSignature.Numerator;
			if (GuiProperty::Input("Time Signature##SyncWindow", numerator, 0.1f, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue), signatureFormatBuffer))
			{
				newSignature = TimeSignature(numerator, newSignature.Denominator);
				executeAddOrUpdate();
			}

			GuiProperty::PropertyLabelValueFunc("", [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const auto buttonWidth = (Gui::GetContentRegionAvailWidth() - style.ItemSpacing.x) / 2.0f;

				if (cursorSitsOnTempoChange)
				{
					if (Gui::Button("Update##SyncWindow", vec2(buttonWidth, 0.0f)))
						executeAddOrUpdate();
				}
				else
				{
					if (Gui::Button("Insert##SyncWindow", vec2(buttonWidth, 0.0f)))
						executeAddOrUpdate();
				}
				Gui::SameLine();

				if (Gui::Button("Remove##SyncWindow", vec2(buttonWidth, 0.0f)))
				{
					if (cursorSitsOnTempoChange)
						undoManager.Execute<RemoveTempoChange>(chart, thisFrameCursorTick);
				}

				Gui::PopStyleVar();
				return false;
			});
		});
	}
}
