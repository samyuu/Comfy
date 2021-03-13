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

	void BPMCalculatorWindow::Gui(Chart& chart, TimeSpan cursorBPMTime, TimelineMetronome& metronome)
	{
		bpmCalculator.SetAutoResetInterval(GlobalUserData.BPMCalculator.AutoResetEnabled ? BPMTapCalculator::DefaultAutoResetInterval : TimeSpan::Zero());
		bpmCalculator.Update();

		const auto tapCount = bpmCalculator.GetTapCount();
		const auto beatTime = TimeSpan::FromSeconds(60.0 / bpmCalculator.GetBPMOnLastTapRound());

		const auto& style = Gui::GetStyle();

		constexpr auto tapKeyBinding = Input::KeyCode_Space, resetKeyBinding = Input::KeyCode_Escape;
		constexpr auto buttonHeight = 66.0f;

		Gui::BeginChild("BPMCalculatorChild", vec2(0.0f, 0.0f), true);
		Gui::BeginColumns("BPMCalculatorColumns", 2, ImGuiColumnsFlags_NoResize);
		{
			{
				const bool tapKeyPressed = (Gui::IsWindowFocused() && Gui::IsKeyPressed(tapKeyBinding, false));
				const bool resetKeyPressed = (Gui::IsWindowFocused() && Gui::IsKeyPressed(resetKeyBinding, false));
				const bool resetKeyDown = (Gui::IsWindowFocused() && Gui::IsKeyDown(resetKeyBinding));

				char tapButtonName[32];
				sprintf_s(tapButtonName, (tapCount == 0) ? "Tap" : (tapCount == 1) ? "First Beat" : "%.2f BPM", bpmCalculator.GetBPMOnLastTapRound());

				const auto lerpDelta = (tapCount == 0) ? 1.0f : static_cast<f32>(bpmCalculator.GetTimeSinceLastTap() / beatTime);
				const auto lerpColor = ImLerp(Gui::GetStyleColorVec4(ImGuiCol_ButtonActive), Gui::GetStyleColorVec4(ImGuiCol_Button), ImSaturate(lerpDelta));

				Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(style.WindowPadding.x + 1.0f, style.ItemSpacing.y));
				Gui::PushStyleColor(ImGuiCol_Button, lerpColor);
				if (tapCount > 0)
				{
					Gui::PushStyleColor(ImGuiCol_ButtonHovered, lerpColor);
					Gui::PushStyleColor(ImGuiCol_ButtonActive, lerpColor);
				}

				if (Gui::ButtonEx(tapButtonName, vec2(Gui::GetContentRegionAvailWidth(), buttonHeight), ImGuiButtonFlags_PressedOnClick) | tapKeyPressed)
				{
					bpmCalculator.Tap();
					if (GlobalUserData.BPMCalculator.ApplyToTempoMap && bpmCalculator.GetTapCount() > 1)
						ExecuteUpdateTempoChangeBPM(chart, cursorBPMTime, bpmCalculator.GetBPMOnLastTapRound());

					PlayTapSoundIfEnabled(metronome);
				}
				Gui::NextColumn();

				if (tapCount > 0)
					Gui::PopStyleColor(2);
				Gui::PopStyleColor(1);


				if (resetKeyDown) Gui::PushStyleColor(ImGuiCol_Button, Gui::GetColorU32(ImGuiCol_ButtonActive));
				Gui::PushItemDisabledAndTextColorIf(tapCount == 0);
				if (Gui::ButtonEx("Reset", vec2(Gui::GetContentRegionAvailWidth(), buttonHeight), ImGuiButtonFlags_PressedOnClickRelease) | resetKeyPressed)
				{
					bpmCalculator.Reset();
					PlayTapSoundIfEnabled(metronome);
				}
				Gui::PopItemDisabledAndTextColorIf(tapCount == 0);
				if (resetKeyDown) Gui::PopStyleColor();
				Gui::NextColumn();

				Gui::PopStyleVar();
			}

			Gui::Separator();
			{
				auto guiBPMText = [](f32 bpm) { if (bpm != 0.0f) { Gui::Text("%.2f BPM", bpm); } else { Gui::TextUnformatted("--.-- BPM"); } };

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Minimum");
				Gui::NextColumn();
				Gui::AlignTextToFramePadding();
				guiBPMText(bpmCalculator.GetBPMOnLastTapMinRound());
				Gui::NextColumn();
				Gui::Separator();

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Maximum");
				Gui::NextColumn();
				Gui::AlignTextToFramePadding();
				guiBPMText(bpmCalculator.GetBPMOnLastTapMaxRound());
				Gui::NextColumn();
				Gui::Separator();

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Running");
				Gui::NextColumn();
				Gui::AlignTextToFramePadding();
				guiBPMText(bpmCalculator.GetRunningBPMRound());
				Gui::NextColumn();
				Gui::Separator();

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Beat Duration");
				Gui::NextColumn();
				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted(beatTime.FormatTime().data());
				Gui::NextColumn();
				Gui::Separator();

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Timing Taps");
				Gui::NextColumn();
				Gui::AlignTextToFramePadding();
				Gui::Text("%d Tap(s)", bpmCalculator.GetTapCount());
				Gui::NextColumn();
			}
		}
		Gui::EndColumns();
		Gui::EndChild();
	}

	void BPMCalculatorWindow::PlayTapSoundIfEnabled(TimelineMetronome& metronome) const
	{
		switch (GlobalUserData.BPMCalculator.TapSoundType)
		{
		case BPMTapSoundType::MetronomeBeat:
		{
			Audio::AudioEngine::GetInstance().EnsureStreamRunning();
			metronome.PlayTickSound(TimeSpan::Zero(), false);
			return;
		}

		case BPMTapSoundType::MetronomeBar:
		{
			Audio::AudioEngine::GetInstance().EnsureStreamRunning();
			metronome.PlayTickSound(TimeSpan::Zero(), true);
			return;
		}

		default:
			return;
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
