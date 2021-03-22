#include "SyncWindow.h"
#include "BeatTick.h"
#include "SortedTempoMap.h"
#include "Editor/Chart/ChartCommands.h"
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
				Gui::TextDisabled("Move by Beat");
				return false;
			}, [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const auto buttonWidth = (Gui::GetContentRegionAvailWidth() - (style.ItemSpacing.x + style.ItemInnerSpacing.x * 2.0f) - 1.0f) / 4.0f;

				auto beatOffsetButton = [&](const char* label, const f64 factor)
				{
					if (Gui::Button(label, vec2(buttonWidth, 0.0f)))
					{
						const auto firstTempo = chart.TempoMap.FindTempoChangeAtTick(BeatTick::Zero()).Tempo;
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

			auto duration = chart.DurationOrDefault();
			if (GuiProperty::PropertyLabelValueFunc("Duration", [&]()
			{
				const auto& style = Gui::GetStyle();
				const f32 buttonWidth = Gui::GetFrameHeight() * 2.0f;

				GuiPropertyRAII::ItemWidth width(std::max(Gui::GetContentRegionAvailWidth() - style.ItemInnerSpacing.x * 2.0f - buttonWidth, 1.0f));
				bool result = Gui::InputFormattedTimeSpan(GuiProperty::Detail::DummyLabel, &duration);

				Gui::SameLine(0.0f, style.ItemInnerSpacing.x);
				if (Gui::Button("Set##Cursor", vec2(std::max(Gui::GetContentRegionAvailWidth(), buttonWidth), 0.0f))) { duration = timeline.GetCursorTime(); result = true; }

				return result;
			}))
			{
				duration = std::max(duration, TimeSpan::FromSeconds(1.0));
				undoManager.Execute<ChangeSongDuration>(chart, duration);
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

			ivec2 sig = { newSignature.Numerator, newSignature.Denominator };
			if (GuiProperty::InputFraction("Time Signature", sig, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue)))
			{
				newSignature = TimeSignature(sig[0], sig[1]);
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
