#include "BPMCalculatorWindow.h"
#include "ChartCommands.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	BPMCalculatorWindow::BPMCalculatorWindow(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void BPMCalculatorWindow::Gui(Chart& chart, TimeSpan cursorBPMTime, TimelineMetronome& metronome, ButtonSoundController& buttonSoundController)
	{
		bpmCalculator.SetAutoResetInterval(GlobalUserData.BPMCalculator.AutoResetEnabled ? BPMTapCalculator::DefaultAutoResetInterval : TimeSpan::Zero());
		bpmCalculator.Update();

		const auto tapCount = bpmCalculator.GetTapCount();
		const auto beatTime = TimeSpan::FromSeconds(60.0 / bpmCalculator.GetBPMOnLastTapRound());

		const auto& style = Gui::GetStyle();
		constexpr f32 buttonHeight = 66.0f;

		Gui::BeginChild("BPMCalculatorChild", vec2(0.0f, 0.0f), true);
		if (Gui::BeginTable("BPMCalculatorTable", 2, ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV))
		{
			const bool isWindowFocused = Gui::IsWindowFocused();
			const bool tapKeyPressed = (isWindowFocused && Input::IsAnyPressed(GlobalUserData.Input.BPMCalculator_Tap, false));
			const bool resetKeyPressed = (isWindowFocused && Input::IsAnyPressed(GlobalUserData.Input.BPMCalculator_Reset, false));
			const bool resetKeyDown = (isWindowFocused && Input::IsAnyDown(GlobalUserData.Input.BPMCalculator_Reset));

			char tapButtonName[32];
			sprintf_s(tapButtonName, (tapCount == 0) ? "Tap" : (tapCount == 1) ? "First Beat" : "%.2f BPM", bpmCalculator.GetBPMOnLastTapRound());

			const auto lerpDelta = (tapCount == 0) ? 1.0f : static_cast<f32>(bpmCalculator.GetTimeSinceLastTap() / beatTime);
			const auto lerpColor = ImLerp(Gui::GetStyleColorVec4(ImGuiCol_ButtonActive), Gui::GetStyleColorVec4(ImGuiCol_Button), ImSaturate(lerpDelta));

			// BUG: Reduce cell padding should only apply to the upper buttons but Tables don't allow changing it midway through
			// Gui::PushStyleVar(ImGuiStyleVar_CellPadding, style.WindowPadding);
			Gui::PushStyleColor(ImGuiCol_Button, lerpColor);
			if (tapCount > 0)
			{
				Gui::PushStyleColor(ImGuiCol_ButtonHovered, lerpColor);
				Gui::PushStyleColor(ImGuiCol_ButtonActive, lerpColor);
			}

			Gui::TableNextRow();
			Gui::TableSetColumnIndex(0);
			if (Gui::ButtonEx(tapButtonName, vec2(Gui::GetContentRegionAvail().x, buttonHeight), ImGuiButtonFlags_PressedOnClick) | tapKeyPressed)
			{
				bpmCalculator.Tap();
				if (GlobalUserData.BPMCalculator.ApplyToTempoMap && bpmCalculator.GetTapCount() > 1)
					ExecuteUpdateTempoChangeBPM(chart, cursorBPMTime, bpmCalculator.GetBPMOnLastTapRound());

				TryPlayTapSound(metronome, buttonSoundController);
			}

			if (tapCount > 0)
				Gui::PopStyleColor(2);
			Gui::PopStyleColor(1);

			if (resetKeyDown) Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_ButtonActive));
			Gui::PushItemDisabledAndTextColorIf(tapCount == 0);

			Gui::TableSetColumnIndex(1);
			if (Gui::ButtonEx("Reset", vec2(Gui::GetContentRegionAvail().x, buttonHeight), ImGuiButtonFlags_PressedOnClickRelease) | resetKeyPressed)
			{
				bpmCalculator.Reset();
				TryPlayTapSound(metronome, buttonSoundController);
			}
			Gui::PopItemDisabledAndTextColorIf(tapCount == 0);
			if (resetKeyDown) Gui::PopStyleColor();

			// Gui::PopStyleVar();

			Gui::Separator();
			{
				auto guiBPMFormatText = [](f32 bpm) { Gui::Text((bpm != 0.0f) ? "%.2f BPM" : "--.-- BPM", bpm); };

				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0); Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Minimum");
				Gui::TableSetColumnIndex(1); Gui::AlignTextToFramePadding(); guiBPMFormatText(bpmCalculator.GetBPMOnLastTapMinRound());
				Gui::Separator();

				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0); Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Maximum");
				Gui::TableSetColumnIndex(1); Gui::AlignTextToFramePadding(); guiBPMFormatText(bpmCalculator.GetBPMOnLastTapMaxRound());
				Gui::Separator();

				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0); Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Running");
				Gui::TableSetColumnIndex(1); Gui::AlignTextToFramePadding(); guiBPMFormatText(bpmCalculator.GetRunningBPMRound());
				Gui::Separator();

				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0); Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Beat Duration");
				Gui::TableSetColumnIndex(1); Gui::AlignTextToFramePadding(); Gui::TextUnformatted(beatTime.FormatTime().data());
				Gui::Separator();

				Gui::TableNextRow();
				Gui::TableSetColumnIndex(0); Gui::AlignTextToFramePadding(); Gui::TextUnformatted("Timing Taps");
				Gui::TableSetColumnIndex(1); Gui::AlignTextToFramePadding(); Gui::Text("%d Tap(s)", bpmCalculator.GetTapCount());
			}

			Gui::EndTable();
		}
		Gui::EndChild();
	}

	void BPMCalculatorWindow::TryPlayTapSound(TimelineMetronome& metronome, ButtonSoundController& buttonSoundController) const
	{
		const auto soundType = GlobalUserData.BPMCalculator.TapSoundType;
		if (soundType == BPMTapSoundType::None)
			return;

		Audio::AudioEngine::GetInstance().EnsureStreamRunning();

		// TODO: Consider handling this differently to account for different volume levels..?
		switch (GlobalUserData.BPMCalculator.TapSoundType)
		{
		case BPMTapSoundType::MetronomeBeat: { metronome.PlayTickSound(TimeSpan::Zero(), false); break; }
		case BPMTapSoundType::MetronomeBar: { metronome.PlayTickSound(TimeSpan::Zero(), true); break; }
		case BPMTapSoundType::ButtonSound: { buttonSoundController.PlayButtonSound(TimeSpan::Zero()); break; }
		case BPMTapSoundType::SlideSound: { buttonSoundController.PlaySlideSound(TimeSpan::Zero()); break; }
		}
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
