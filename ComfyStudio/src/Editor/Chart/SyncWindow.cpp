#include "SyncWindow.h"
#include "BeatTick.h"
#include "SortedTempoMap.h"
#include "Editor/Chart/ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		std::array<Tempo, 2> FindMinMaxTempo(const SortedTempoMap& tempoMap)
		{
			const f32 initialBPM = tempoMap.GetRawViewAt(0).Tempo.value_or(TempoChange::DefaultTempo).BeatsPerMinute;
			f32 minBPM = initialBPM, maxBPM = initialBPM;

			for (const auto& tempoChange : tempoMap.GetRawView())
			{
				if (!tempoChange.Tempo.has_value())
					continue;
				if (tempoChange.Tempo->BeatsPerMinute < minBPM) minBPM = tempoChange.Tempo->BeatsPerMinute;
				if (tempoChange.Tempo->BeatsPerMinute > maxBPM) maxBPM = tempoChange.Tempo->BeatsPerMinute;
			}

			return { Tempo(minBPM), Tempo(maxBPM) };
		}
	}

	SyncWindow::SyncWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void SyncWindow::Gui(Chart& chart, TargetTimeline& timeline)
	{
		GuiPropertyRAII::PropertyValueColumns columns;

		GuiProperty::TreeNode("Song Sync", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			// NOTE: Negative to visually match the drag direction with that of the waveform timeline position
			constexpr f32 offsetDragSpeed = -1.0f;

			f32 songOffsetMS = static_cast<f32>(chart.SongOffset.TotalMilliseconds());
			if (GuiProperty::Input("Song Offset##SyncWindow", songOffsetMS, offsetDragSpeed, {}, "%.2f ms"))
				undoManager.Execute<ChangeSongOffset>(chart, TimeSpan::FromMilliseconds(songOffsetMS));

			// HACK: Manual GuiProperty::Detail accesses here to be able to change ValueFunc only text color
			{
				constexpr std::string_view movieOffsetLabel = "Movie Offset##SyncWindow";
				Gui::PushID(Gui::StringViewStart(movieOffsetLabel), Gui::StringViewEnd(movieOffsetLabel));

				f32 movieOffsetMS = static_cast<f32>(chart.MovieOffset.TotalMilliseconds());
				const bool movieOffsetChanged = GuiProperty::PropertyFuncValueFunc([&]
				{
					return GuiProperty::Detail::DragTextT<f32>(movieOffsetLabel, movieOffsetMS, offsetDragSpeed, nullptr, nullptr, 0.0f);
				}, [&]
				{
					if (chart.MovieFileName.empty()) Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
					const bool valueChanged = GuiProperty::Detail::InputVec1DragBaseValueFunc(movieOffsetMS, offsetDragSpeed, {}, "%.2f ms");
					if (chart.MovieFileName.empty()) Gui::PopStyleColor();
					return valueChanged;
				});

				if (movieOffsetChanged)
					undoManager.Execute<ChangeMovieOffset>(chart, TimeSpan::FromMilliseconds(movieOffsetMS));

				Gui::PopID();
			}

			GuiProperty::PropertyFuncValueFunc([&]
			{
				Gui::TextUnformatted("Move by Beat");
				return false;
			}, [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const f32 buttonWidth = (Gui::GetContentRegionAvail().x - (style.ItemSpacing.x + style.ItemInnerSpacing.x * 2.0f) - 1.0f) / 4.0f;

				auto beatOffsetButton = [&](const char* label, const f64 factor)
				{
					if (Gui::Button(label, vec2(buttonWidth, 0.0f)))
					{
						const Tempo firstTempo = chart.TempoMap.FindNewOrInheritedAt(0).Tempo;
						const TimeSpan beatDuration = TimeSpan::FromSeconds(60.0 / firstTempo.BeatsPerMinute);

						undoManager.Execute<ChangeSongOffset>(chart, chart.SongOffset + (beatDuration * factor));
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
			if (GuiProperty::PropertyFuncValueFunc([&]
			{
				f32 durationMS = static_cast<f32>(duration.TotalMilliseconds());
				f32 durationMSMin = static_cast<f32>(TimeSpan::FromSeconds(1.0).TotalMilliseconds());
				if (GuiProperty::Detail::DragTextT<f32>("Duration", durationMS, 1000.0f, &durationMSMin, nullptr, 0.0f))
				{
					duration = TimeSpan::FromMilliseconds(durationMS);
					return true;
				}
				return false;
			}, [&]
			{
				const auto& style = Gui::GetStyle();
				const f32 buttonWidth = Gui::GetFrameHeight() * 2.0f;

				GuiPropertyRAII::ItemWidth width(Max(Gui::GetContentRegionAvail().x - style.ItemInnerSpacing.x * 2.0f - buttonWidth, 1.0f));
				bool result = Gui::InputFormattedTimeSpan(GuiProperty::Detail::DummyLabel, &duration, {}, ImGuiInputTextFlags_AutoSelectAll);

				Gui::SameLine(0.0f, style.ItemInnerSpacing.x);
				if (Gui::Button("Set##Cursor", vec2(Max(Gui::GetContentRegionAvail().x, buttonWidth), 0.0f))) { duration = timeline.GetCursorTime(); result = true; }

				return result;
			}))
			{
				duration = Max(duration, TimeSpan::FromSeconds(1.0));
				undoManager.Execute<ChangeSongDuration>(chart, duration);
			}
		});

		lastFrameCursorTick = thisFrameCursorTick;
		thisFrameCursorTick = timeline.RoundTickToGrid(timeline.GetCursorTick());

		NewOrInheritedTempoChange newOrInheritedTempoChangeAtCursor = chart.TempoMap.FindNewOrInheritedAtTick(thisFrameCursorTick);
		const TempoChange& tempoChangeAtCursor = chart.TempoMap.GetRawViewAt(newOrInheritedTempoChangeAtCursor.IndexWithinTempoMap);

		const bool cursorSitsOnTempoChange = (tempoChangeAtCursor.Tick == thisFrameCursorTick);

		const auto[minTempo, maxTempo] = FindMinMaxTempo(chart.TempoMap);
		char rhythmNodeValueBuffer[64];
		const auto rhythmNodeValueView = std::string_view(rhythmNodeValueBuffer, (minTempo.BeatsPerMinute == maxTempo.BeatsPerMinute) ?
			sprintf_s(rhythmNodeValueBuffer, "(%.2f BPM)", minTempo.BeatsPerMinute) :
			sprintf_s(rhythmNodeValueBuffer, "(%.2f - %.2f BPM)", minTempo.BeatsPerMinute, maxTempo.BeatsPerMinute));

		GuiProperty::TreeNode("Chart Rhythm", rhythmNodeValueView, ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			auto executeAddOrUpdate = [&](std::optional<Tempo> newTempo, std::optional<FlyingTimeFactor> newFlyingTime, std::optional<TimeSignature> newSignature)
			{
				// HACK: Moving the cursor on the timeline (= losing focus on the same frame) while having a textbox focused
				//		 would otherwise incorrectly insert an additional tempo change at the new cursor location
				if (thisFrameCursorTick != lastFrameCursorTick)
					return;

				if (cursorSitsOnTempoChange)
				{
					if (!newTempo.has_value()) newTempo = tempoChangeAtCursor.Tempo;
					if (!newFlyingTime.has_value()) newFlyingTime = tempoChangeAtCursor.FlyingTime;
					if (!newSignature.has_value()) newSignature = tempoChangeAtCursor.Signature;
					undoManager.Execute<UpdateTempoChange>(chart, TempoChange(thisFrameCursorTick, newTempo, newFlyingTime, newSignature));
				}
				else
				{
					undoManager.Execute<AddTempoChange>(chart, TempoChange(thisFrameCursorTick, newTempo, newFlyingTime, newSignature));
				}
			};

			if (GuiProperty::Input("Tempo##SyncWindow", newOrInheritedTempoChangeAtCursor.Tempo.BeatsPerMinute, 1.0f, vec2(Tempo::MinBPM, Tempo::MaxBPM), "%.2f BPM"))
				executeAddOrUpdate(newOrInheritedTempoChangeAtCursor.Tempo, {}, {});

			f32 flyingTimeFactor = ToPercent(newOrInheritedTempoChangeAtCursor.FlyingTime.Factor);
			if (GuiProperty::Input("Flying Time", flyingTimeFactor, 1.0f, ToPercent(vec2(FlyingTimeFactor::Min, FlyingTimeFactor::Max)), "%.2f %%"))
				executeAddOrUpdate({}, FlyingTimeFactor(FromPercent(flyingTimeFactor)), {});

			ivec2 sig = { newOrInheritedTempoChangeAtCursor.Signature.Numerator, newOrInheritedTempoChangeAtCursor.Signature.Denominator };
			if (GuiProperty::InputFraction("Time Signature", sig, ivec2(TimeSignature::MinValue, TimeSignature::MaxValue)))
				executeAddOrUpdate({}, {}, TimeSignature(sig[0], sig[1]));

			GuiProperty::PropertyLabelValueFunc("", [&]
			{
				const auto& style = Gui::GetStyle();
				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));
				const f32 buttonWidth = (Gui::GetContentRegionAvail().x - style.ItemSpacing.x) / 2.0f;

				// TODO: What exactly should happen here (?)
				if (Gui::Button(cursorSitsOnTempoChange ? "Update##SyncWindow" : "Insert##SyncWindow", vec2(buttonWidth, 0.0f)))
					executeAddOrUpdate(newOrInheritedTempoChangeAtCursor.Tempo, newOrInheritedTempoChangeAtCursor.FlyingTime, newOrInheritedTempoChangeAtCursor.Signature);

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
