#include "BPMCalculatorWindow.h"
#include "ChartCommands.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	BPMCalculatorWindow::BPMCalculatorWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void BPMCalculatorWindow::Gui(Chart& chart, TimeSpan cursorBPMTime)
	{
		bpmCalculator.Update();
		const auto tapCount = bpmCalculator.GetTapCount();

		constexpr auto tapKeyBinding = Input::KeyCode_Space, resetKeyBinding = Input::KeyCode_Escape;

		const bool tapKeyPressed = (Gui::IsWindowFocused() && Gui::IsKeyPressed(tapKeyBinding, false));
		const bool tapKeyDown = (Gui::IsWindowFocused() && Gui::IsKeyDown(tapKeyBinding));

		const bool resetKeyPressed = (Gui::IsWindowFocused() && Gui::IsKeyPressed(resetKeyBinding, false));
		const bool resetKeyDown = (Gui::IsWindowFocused() && Gui::IsKeyDown(resetKeyBinding));

		const auto& style = Gui::GetStyle();
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.ItemInnerSpacing.x, style.ItemSpacing.y));

		const auto fullButtonWidth = Gui::GetContentRegionAvailWidth();
		const auto halfButtonWidth = (fullButtonWidth - style.ItemSpacing.x) / 2.0f;
		const auto buttonHeight = 72.0f;

		char tapButtonName[32];
		sprintf_s(tapButtonName, (tapCount == 0) ? "Tap" : (tapCount == 1) ? "First Beat" : "%.2f BPM", bpmCalculator.GetBPMOnLastTapRound());

		const auto beatTime = TimeSpan::FromSeconds(60.0 / bpmCalculator.GetBPMOnLastTapRound());
		const auto lerpDelta = (tapCount == 0) ? 1.0f : static_cast<f32>(bpmCalculator.GetTimeSinceLastTap() / beatTime);
		const auto lerpColor = ImLerp(Gui::GetStyleColorVec4(ImGuiCol_ButtonActive), Gui::GetStyleColorVec4(ImGuiCol_Button), ImSaturate(lerpDelta));

		Gui::PushStyleColor(ImGuiCol_Button, lerpColor);
		if (tapCount > 0)
		{
			Gui::PushStyleColor(ImGuiCol_ButtonHovered, lerpColor);
			Gui::PushStyleColor(ImGuiCol_ButtonActive, lerpColor);
		}

		if (Gui::ButtonEx(tapButtonName, vec2(halfButtonWidth, buttonHeight), ImGuiButtonFlags_PressedOnClick) | tapKeyPressed)
		{
			bpmCalculator.Tap();

			if (applyTapToTempoMap && bpmCalculator.GetTapCount() > 1)
				ExecuteUpdateTempoChangeBPM(chart, cursorBPMTime, bpmCalculator.GetBPMOnLastTapRound());
		}

		Gui::PopStyleColor((tapCount > 0) ? 3 : 1);

		Gui::SameLine();

		if (resetKeyDown)
			Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_ButtonActive));

		Gui::PushItemDisabledAndTextColorIf(tapCount == 0);
		if (Gui::ButtonEx("Reset", vec2(halfButtonWidth, buttonHeight), ImGuiButtonFlags_PressedOnClickRelease) | resetKeyPressed)
		{
			bpmCalculator.Reset();
		}
		Gui::PopItemDisabledAndTextColorIf(tapCount == 0);

		if (resetKeyDown)
			Gui::PopStyleColor();

		Gui::PopStyleVar();

		Gui::WindowContextMenu("BPMCalculatorContextMenu", [&]
		{
			Gui::Text("BPM Calculator:");
			Gui::Separator();
			bool autoResetEnabled = (bpmCalculator.GetAutoResetInterval() > TimeSpan::Zero());
			if (Gui::MenuItem("Auto Reset Enabled", nullptr, &autoResetEnabled))
				bpmCalculator.SetAutoResetInterval(autoResetEnabled ? BPMTapCalculator::DefaultAutoResetInterval : TimeSpan::Zero());
			Gui::MenuItem("Apply to Tempo Map", nullptr, &applyTapToTempoMap);
		});

		Gui::BeginColumns("BPMCalculatorColumns", 2, ImGuiColumnsFlags_NoBorder);
		{
			auto bpmText = [](f32 bpm)
			{
				if (bpm != 0.0f)
					Gui::Text("%.2f BPM", bpm);
				else
					Gui::Text("--.-- BPM");
			};

			Gui::TextUnformatted("Minimum");
			Gui::NextColumn();
			bpmText(bpmCalculator.GetBPMOnLastTapMinRound());
			Gui::NextColumn();

			Gui::TextUnformatted("Maximum");
			Gui::NextColumn();
			bpmText(bpmCalculator.GetBPMOnLastTapMaxRound());
			Gui::NextColumn();

			Gui::TextUnformatted("Running");
			Gui::NextColumn();
			bpmText(bpmCalculator.GetRunningBPMRound());
			Gui::NextColumn();

			Gui::TextUnformatted("Beat Duration");
			Gui::NextColumn();
			Gui::TextUnformatted(beatTime.FormatTime().data());
			Gui::NextColumn();

			Gui::TextUnformatted("Timing Taps");
			Gui::NextColumn();
			Gui::Text("%d Tap(s)", bpmCalculator.GetTapCount());
			Gui::NextColumn();
		}
		Gui::EndColumns();
	}

	void BPMCalculatorWindow::ExecuteUpdateTempoChangeBPM(Chart& chart, TimeSpan cursorBPMTime, Tempo updatedTempo) const
	{
		const auto& tempoChange = chart.TempoMap.FindTempoChangeAtTick(chart.TimelineMap.GetTickAt(cursorBPMTime));

		undoManager.Execute<UpdateTempoChange>(chart, TempoChange(
			tempoChange.Tick,
			std::clamp(updatedTempo.BeatsPerMinute, Tempo::MinBPM, Tempo::MaxBPM),
			tempoChange.Signature));
	}
}
