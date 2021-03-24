#include "TargetPathTool.h"
#include "CardinalDirection.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Editor/Chart/KeyBindings.h"
#include "Core/ComfyStudioSettings.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	void TargetPathTool::OnSelected()
	{
	}

	void TargetPathTool::OnDeselected()
	{
	}

	void TargetPathTool::PreRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetPathTool::PostRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetPathTool::OnContextMenuGUI(Chart& chart)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });

		if (Gui::MenuItem("Invert Target Frequencies", Input::GetKeyCodeName(KeyBindings::PathToolInvertFrequencies), false, (selectionCount > 0)))
			InvertSelectedTargetFrequencies(undoManager, chart);
		Gui::Separator();

		if (Gui::MenuItem("Interpolate Angles Clockwise", Input::GetKeyCodeName(KeyBindings::PathToolInterpolateAnglesClockwise), false, (selectionCount > 0)))
			InterpolateSelectedTargetAngles(undoManager, chart, true);
		if (Gui::MenuItem("Interpolate Angles Counterclockwise", Input::GetKeyCodeName(KeyBindings::PathToolInterpolateAnglesCounterclockwise), false, (selectionCount > 0)))
			InterpolateSelectedTargetAngles(undoManager, chart, false);

		if (Gui::MenuItem("Interpolate Distances", Input::GetKeyCodeName(KeyBindings::PathToolInterpolateDistances), false, (selectionCount > 0)))
			InterpolateSelectedTargetDistances(undoManager, chart);

		Gui::Separator();

		if (Gui::BeginMenu("Angle Increment Settings"))
		{
			Gui::Checkbox("Fixed Step Increment", &angleIncrement.UseFixedStepIncrement);
			Gui::SameLineHelpMarkerRightAlign(
				"Instead of scaling the angle increment by the beat time distance,\n"
				"increment angles per target regardless of their time difference\n"
				"(Use with caution!)"
			);

			Gui::Checkbox("Apply to Chain Slide Fragments", &angleIncrement.ApplyToChainSlides);
			Gui::SameLineHelpMarkerRightAlign(
				"Increment angle with each chain slide fragment instead of only the starting piece.\n"
				"In most cases chain slides should make use of uniform angles\n"
				"(Use with caution!)"
			);

			static constexpr f32 min = 0.0f, max = 90.0f, step = 1.0f, stepFast = 5.0f;
			auto clampIncrementStep = [](f32& inOut) { inOut = std::clamp(inOut, min, max); };

			if (angleIncrement.UseFixedStepIncrement)
			{
				if (Gui::InputFloat("##IncrementPerTarget", &angleIncrement.FixedStepIncrementPerTarget, step, stepFast, ("%.2f" DEGREE_SIGN " per Target")))
					clampIncrementStep(angleIncrement.FixedStepIncrementPerTarget);
			}
			else
			{
				if (Gui::InputFloat("##IncrementPerBeat", &angleIncrement.IncrementPerBeat, step, stepFast, ("%.2f" DEGREE_SIGN " per Beat")))
					clampIncrementStep(angleIncrement.IncrementPerBeat);
				if (Gui::InputFloat("##IncrementPerBeatDiagonal", &angleIncrement.IncrementPerBeatDiagonal, step, stepFast, ("%.2f" DEGREE_SIGN " per Beat (Diagonal)")))
					clampIncrementStep(angleIncrement.IncrementPerBeatDiagonal);
			}

			static constexpr AngleIncrementData defaultIncrementData = { 2.0f, 10.0f }, defaultIncrementDataSmall = { 1.0f, 8.0f };
			const bool isDefault = (angleIncrement == defaultIncrementData), isDefaultSmall = (angleIncrement == defaultIncrementDataSmall);

			Gui::PushItemDisabledAndTextColorIf(isDefault);
			if (Gui::Button("Set Default", vec2(Gui::GetContentRegionAvailWidth() * 0.5f, 0.0f))) angleIncrement = defaultIncrementData;
			Gui::PopItemDisabledAndTextColorIf(isDefault);

			Gui::SameLine(0.0f, 2.0f);

			Gui::PushItemDisabledAndTextColorIf(isDefaultSmall);
			if (Gui::Button("Set Default Small", vec2(Gui::GetContentRegionAvailWidth(), 0.0f))) angleIncrement = defaultIncrementDataSmall;
			Gui::PopItemDisabledAndTextColorIf(isDefaultSmall);

			Gui::EndMenu();
		}

		// TODO: Implement keybinding string conversion with support for modifiers
		static_assert(KeyBindings::PathToolApplyAngleIncrementsPositive == Input::KeyCode_F);
		static_assert(KeyBindings::PathToolApplyAngleIncrementsNegative == Input::KeyCode_V);

		if (Gui::MenuItem("Apply Angle Increment Positive", Input::GetKeyCodeName(KeyBindings::PathToolApplyAngleIncrementsPositive), false, (selectionCount > 0)))
			ApplySelectedTargetAngleIncrements(undoManager, chart, +1.0f, false);
		if (Gui::MenuItem("Apply Angle Increment Positive (Back)", "Alt + F", false, (selectionCount > 0)))
			ApplySelectedTargetAngleIncrements(undoManager, chart, +1.0f, true);

		if (Gui::MenuItem("Apply Angle Increment Negative", Input::GetKeyCodeName(KeyBindings::PathToolApplyAngleIncrementsNegative), false, (selectionCount > 0)))
			ApplySelectedTargetAngleIncrements(undoManager, chart, -1.0f, false);
		if (Gui::MenuItem("Apply Angle Increment Negative (Back)", "Alt + V", false, (selectionCount > 0)))
			ApplySelectedTargetAngleIncrements(undoManager, chart, -1.0f, true);

		Gui::Separator();
	}

	void TargetPathTool::OnOverlayGUI(Chart& chart)
	{
	}

	void TargetPathTool::PreRenderGUI(Chart& chart, ImDrawList& drawList)
	{
	}

	void TargetPathTool::PostRenderGUI(Chart& chart, ImDrawList& drawList)
	{
		DrawTargetAngleGuides(chart, drawList);
		DrawAngleDragGuide(chart, drawList);
	}

	void TargetPathTool::UpdateInput(Chart& chart)
	{
		UpdateKeyboardKeyBindingsInput(chart);
		UpdateMouseAngleScrollInput(chart);
		UpdateMouseAngleDragInput(chart);
	}

	const char* TargetPathTool::GetName() const
	{
		return "Path Tool";
	}

	void TargetPathTool::DrawTargetAngleGuides(Chart& chart, ImDrawList& drawList)
	{
		const i32 maxPathsToDraw = GlobalUserData.System.Gui.TargetButtonPathMaxCount;
		i32 pathDrawCount = 0;

		if (angleDrag.Active || angleScroll.Active)
		{
			const auto* dragTarget = IndexOrNull(angleDrag.Active ? angleDrag.TargetIndex : angleScroll.TargetIndex, chart.Targets);

			for (auto& target : chart.Targets)
			{
				if (target.IsSelected && &target != dragTarget)
				{
					DrawStraightButtonAngleLine(renderWindow, drawList, Rules::TryGetProperties(target), GetButtonTypeColorU32(target.Type, 0x76), 2.0f);
					if (pathDrawCount++ >= maxPathsToDraw)
						break;
				}
			}

			// NOTE: For readibility sake always draw the main drag arrow on top
			if (dragTarget != nullptr)
				DrawCurvedButtonPathLineArrowHeads(renderWindow, drawList, Rules::TryGetProperties(*dragTarget), GetButtonTypeColorU32(dragTarget->Type, 0xD6), 2.0f);
		}
		else
		{
			for (auto& target : chart.Targets)
			{
				if (target.IsSelected)
				{
					DrawCurvedButtonPathLine(renderWindow, drawList, Rules::TryGetProperties(target), GetButtonTypeColorU32(target.Type, 0xD6), 2.0f);
					if (pathDrawCount++ >= maxPathsToDraw)
						break;
				}
			}
		}
	}

	void TargetPathTool::DrawAngleDragGuide(Chart& chart, ImDrawList& drawList)
	{
		if (!angleDrag.Active && !angleScroll.Active)
			return;

		const auto* dragTarget = IndexOrNull(angleDrag.Active ? angleDrag.TargetIndex : angleScroll.TargetIndex, chart.Targets);
		if (dragTarget == nullptr)
			return;

		const auto targetProperties = Rules::TryGetProperties(*dragTarget);
		const vec2 screenPosition = renderWindow.TargetAreaToScreenSpace(targetProperties.Position);
		const f32 cameraZoom = renderWindow.GetCamera().Zoom;

		const u32 buttonTypeColor = GetButtonTypeColorU32(dragTarget->Type, 0x80);

		drawList.AddLine(
			screenPosition,
			screenPosition + vec2(0.0f, -Rules::TickToDistance(BeatTick::FromBars(1) / 6) * cameraZoom),
			buttonTypeColor,
			2.0f);

		const f32 radius = Rules::TickToDistance(BeatTick::FromBars(1) / 8) * cameraZoom;
		const f32 angle = Rules::NormalizeAngle(targetProperties.Angle) - 90.0f;

		drawList.PathArcTo(screenPosition, radius - 0.5f, glm::radians(-90.0f), glm::radians(angle), 32);
		drawList.PathStroke(buttonTypeColor, false, 2.0f);

		char buffer[64];
		const auto bufferView = std::string_view(buffer, sprintf_s(buffer, ("%.2f" DEGREE_SIGN), targetProperties.Angle));

		const vec2 textPadding = vec2(3.0f, 1.0f);
		const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView)) + textPadding;
		const vec2 textPos = renderWindow.TargetAreaToScreenSpace(
			targetProperties.Position + vec2(Rules::TickToDistance(BeatTick::FromBars(1) / 10)) * vec2(1.0f, -1.0f)) - (textSize / 2.0f);

		// TODO: Turn into tooltip (?) just like for the Position Tool
		const u32 dimColor = ImColor(0.1f, 0.1f, 0.1f, 0.85f);
		drawList.AddRectFilled(textPos, textPos + textSize, dimColor);
		drawList.AddText(textPos + (textPadding / 2.0f), Gui::GetColorU32(ImGuiCol_Text), Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView));
	}

	void TargetPathTool::UpdateKeyboardKeyBindingsInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			if (Gui::IsKeyPressed(KeyBindings::PathToolInvertFrequencies, false))
				InvertSelectedTargetFrequencies(undoManager, chart);
			if (Gui::IsKeyPressed(KeyBindings::PathToolInterpolateAnglesClockwise, false))
				InterpolateSelectedTargetAngles(undoManager, chart, true);
			if (Gui::IsKeyPressed(KeyBindings::PathToolInterpolateAnglesCounterclockwise, false))
				InterpolateSelectedTargetAngles(undoManager, chart, false);
			if (Gui::IsKeyPressed(KeyBindings::PathToolInterpolateDistances, false))
				InterpolateSelectedTargetDistances(undoManager, chart);

			const bool backwards = Gui::GetIO().KeyAlt;
			if (Gui::IsKeyPressed(KeyBindings::PathToolApplyAngleIncrementsPositive, false))
				ApplySelectedTargetAngleIncrements(undoManager, chart, +1.0f, backwards);
			if (Gui::IsKeyPressed(KeyBindings::PathToolApplyAngleIncrementsNegative, false))
				ApplySelectedTargetAngleIncrements(undoManager, chart, -1.0f, backwards);
		}
	}

	void TargetPathTool::UpdateMouseAngleScrollInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			if (const auto& io = Gui::GetIO(); io.MouseWheel != 0.0f)
			{
				// TODO: Make modifiers configurable too (?)
				const f32 angleIncrement =
					io.KeyShift ? GlobalUserData.PathTool.AngleMouseScrollRough :
					io.KeyAlt ? GlobalUserData.PathTool.AngleMouseScrollPrecise : GlobalUserData.PathTool.AngleMouseScrollStep;

				IncrementSelectedTargetAnglesBy(undoManager, chart, (angleIncrement * io.MouseWheel * GlobalUserData.PathTool.AngleMouseScrollDirection));
				angleScroll.LastScroll.Restart();
			}
			else if (Gui::IsMouseClicked(2, true))
			{
				// NOTE: Middle mouse surving as a way to quickly preview angles, especially useful when holding down while cycling through different targets
				angleScroll.LastScroll.Restart();
			}
		}

		constexpr auto activeScrollThreshold = TimeSpan::FromSeconds(0.75);
		angleScroll.Active = angleScroll.LastScroll.IsRunning() && angleScroll.LastScroll.GetElapsed() <= activeScrollThreshold;
		angleScroll.TargetIndex = angleScroll.Active ? static_cast<i32>(FindIndexOf(chart.Targets, [](auto& t) { return t.IsSelected; })) : -1;
	}

	void TargetPathTool::UpdateMouseAngleDragInput(Chart& chart)
	{
		if (angleDrag.Active && Gui::IsMouseReleased(0))
			undoManager.DisallowMergeForLastCommand();

		if (!Gui::IsWindowFocused() || Gui::IsMouseReleased(0))
			angleDrag = {};

		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			if (Gui::IsMouseClicked(0) && !Gui::IsAnyItemHovered())
			{
				angleDrag.UseLastTarget = Gui::GetIO().KeyAlt;

				const size_t targetIndex = angleDrag.UseLastTarget ?
					FindLastIndexOf(chart.Targets, [](auto& t) { return t.IsSelected; }) :
					FindIndexOf(chart.Targets, [](auto& t) { return t.IsSelected; });

				if (InBounds(targetIndex, chart.Targets))
				{
					angleDrag.Active = true;
					angleDrag.StartMouse = Gui::GetMousePos();
					angleDrag.StartTarget = renderWindow.TargetAreaToScreenSpace(Rules::TryGetProperties(chart.Targets[targetIndex]).Position);
					angleDrag.TargetIndex = static_cast<i32>(targetIndex);
					undoManager.DisallowMergeForLastCommand();
				}
			}
		}

		if (angleDrag.Active)
		{
			Gui::SetActiveID(Gui::GetID(&angleDrag), Gui::GetCurrentWindow());
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);

			// TODO: Make user configurable (?)
			angleDrag.RoughStep = Gui::GetIO().KeyShift;
			angleDrag.PreciseStep = Gui::GetIO().KeyAlt;

			angleDrag.EndMouse = Gui::GetMousePos();
			angleDrag.TargetMouseDirection = glm::normalize(angleDrag.EndMouse - angleDrag.StartTarget);
			angleDrag.DegreesTargetAngle = glm::degrees(glm::atan(angleDrag.TargetMouseDirection.y, angleDrag.TargetMouseDirection.x)) + 90.0f;

			constexpr f32 distanceActionThreshold = 3.0f;
			if (!angleDrag.MovedFarEnoughFromStart && glm::distance(angleDrag.StartMouse, angleDrag.EndMouse) > distanceActionThreshold)
				angleDrag.MovedFarEnoughFromStart = true;

			if (angleDrag.MovedFarEnoughFromStart)
			{
				const f32 snap =
					angleDrag.RoughStep ? GlobalUserData.PathTool.AngleMouseSnapRough :
					angleDrag.PreciseStep ? GlobalUserData.PathTool.AngleMouseSnapPrecise : GlobalUserData.PathTool.AngleMouseSnap;

				angleDrag.DegreesTargetAngle = Rules::NormalizeAngle(glm::round(angleDrag.DegreesTargetAngle / snap) * snap);

				constexpr f32 distanceThreshold = 4.0f;
				if (glm::distance(angleDrag.StartTarget, angleDrag.EndMouse) > distanceThreshold)
					SetSelectedTargetAnglesTo(undoManager, chart, angleDrag.DegreesTargetAngle);
			}
		}
	}

	void TargetPathTool::IncrementSelectedTargetAnglesBy(Undo::UndoManager& undoManager, Chart& chart, f32 increment)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Angle = Rules::NormalizeAngle(Rules::TryGetProperties(target).Angle + increment);
		}

		undoManager.Execute<ChangeTargetListAngles>(chart, std::move(targetData));
	}

	void TargetPathTool::SetSelectedTargetAnglesTo(Undo::UndoManager& undoManager, Chart& chart, f32 newAngle)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Angle = newAngle;
		}

		undoManager.Execute<ChangeTargetListAngles>(chart, std::move(targetData));
	}

	void TargetPathTool::InvertSelectedTargetFrequencies(Undo::UndoManager& undoManager, Chart& chart)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const f32 targetFrequency = Rules::TryGetProperties(target).Frequency;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Frequency = (targetFrequency == 0.0f) ? 0.0f : (targetFrequency * -1.0f);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InvertTargetListFrequencies>(chart, std::move(targetData));
	}

	void TargetPathTool::InterpolateSelectedTargetAngles(Undo::UndoManager& undoManager, Chart& chart, bool clockwise)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstFoundTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		const auto& lastFoundTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [](auto& t) { return t.IsSelected; });
		if (firstFoundTarget.Tick == lastFoundTarget.Tick)
			return;

		const f32 startAngle = Rules::TryGetProperties(firstFoundTarget).Angle;
		const f32 endAngle = clockwise ? Rules::TryGetProperties(lastFoundTarget).Angle : Rules::TryGetProperties(lastFoundTarget).Angle - 360.0f;

		const i32 startTicks = firstFoundTarget.Tick.Ticks();
		const i32 endTicks = lastFoundTarget.Tick.Ticks();
		const f32 tickSpanReciprocal = 1.0f / static_cast<f32>(endTicks - startTicks);

		std::vector<InterpolateTargetListAngles::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const f32 t = static_cast<f32>(target.Tick.Ticks() - startTicks) * tickSpanReciprocal;
			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Angle = Rules::NormalizeAngle(((1.0f - t) * startAngle) + (t * endAngle));
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListAngles>(chart, std::move(targetData));
	}

	void TargetPathTool::InterpolateSelectedTargetDistances(Undo::UndoManager& undoManager, Chart& chart)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstFoundTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		const auto& lastFoundTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [](auto& t) { return t.IsSelected; });
		if (firstFoundTarget.Tick == lastFoundTarget.Tick)
			return;

		const f32 startDistance = Rules::TryGetProperties(firstFoundTarget).Distance;
		const f32 endDistance = Rules::TryGetProperties(lastFoundTarget).Distance;

		const i32 startTicks = firstFoundTarget.Tick.Ticks();
		const i32 endTicks = lastFoundTarget.Tick.Ticks();
		const f32 tickSpanReciprocal = 1.0f / static_cast<f32>(endTicks - startTicks);

		std::vector<InterpolateTargetListDistances::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const f32 t = static_cast<f32>(target.Tick.Ticks() - startTicks) * tickSpanReciprocal;
			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Distance = ((1.0f - t) * startDistance) + (t * endDistance);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListDistances>(chart, std::move(targetData));
	}

	void TargetPathTool::ApplySelectedTargetAngleIncrements(Undo::UndoManager& undoManager, Chart& chart, f32 direction, bool backwards)
	{
		assert(std::isnormal(direction));

		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListAngles::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue = Rules::TryGetProperties(target);
		}

		auto applyUsingForwardOrReverseIterators = [this, &chart, direction](auto beginIt, auto endIt)
		{
			auto prevDataIt = beginIt;
			auto thisDataIt = beginIt + 1;

			if (angleIncrement.UseFixedStepIncrement)
			{
				while (thisDataIt != endIt)
				{
					const auto& thisTarget = chart.Targets[chart.Targets.FindIndex(thisDataIt->ID)];
					const f32 finalIncrement = (!angleIncrement.ApplyToChainSlides && thisTarget.Flags.IsChain && !thisTarget.Flags.IsChainStart) ?
						0.0f : (-angleIncrement.FixedStepIncrementPerTarget * direction);

					thisDataIt->NewValue.Angle = Rules::NormalizeAngle(prevDataIt->NewValue.Angle + finalIncrement);

					prevDataIt++;
					thisDataIt++;
				}
			}
			else
			{
				while (thisDataIt != endIt)
				{
					const auto& prevTarget = chart.Targets[chart.Targets.FindIndex(prevDataIt->ID)];
					const auto& thisTarget = chart.Targets[chart.Targets.FindIndex(thisDataIt->ID)];

					const auto tickDifference = (prevTarget.Tick - thisTarget.Tick);

					const vec2 directionToLastTarget = glm::normalize(prevDataIt->NewValue.Position - thisDataIt->NewValue.Position);
					const f32 angleToLastTarget = glm::degrees(glm::atan(directionToLastTarget.y, directionToLastTarget.x));
					const bool isDiagonal = IsIntercardinal(AngleToNearestCardinal(angleToLastTarget));

					const f32 incrementPerBeat = (isDiagonal ? angleIncrement.IncrementPerBeatDiagonal : angleIncrement.IncrementPerBeat);
					const f32 finalIncrement = (!angleIncrement.ApplyToChainSlides && thisTarget.Flags.IsChain && !thisTarget.Flags.IsChainStart) ?
						0.0f : (tickDifference.BeatsFraction() * incrementPerBeat * direction);

					thisDataIt->NewValue.Angle = Rules::NormalizeAngle(prevDataIt->NewValue.Angle + finalIncrement);

					prevDataIt++;
					thisDataIt++;
				}
			}
		};

		if (backwards)
			applyUsingForwardOrReverseIterators(targetData.rbegin(), targetData.rend());
		else
			applyUsingForwardOrReverseIterators(targetData.begin(), targetData.end());

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<ApplyTargetListAngleIncrements>(chart, std::move(targetData));
	}

	bool TargetPathTool::AngleIncrementData::operator==(const AngleIncrementData& other) const
	{
		return
			(IncrementPerBeat == other.IncrementPerBeat) &&
			(IncrementPerBeatDiagonal == other.IncrementPerBeatDiagonal) &&
			(FixedStepIncrementPerTarget == other.FixedStepIncrementPerTarget) &&
			(UseFixedStepIncrement == other.UseFixedStepIncrement) &&
			(ApplyToChainSlides == other.ApplyToChainSlides);
	}

	bool TargetPathTool::AngleIncrementData::operator!=(const AngleIncrementData& other) const
	{
		return !(*this == other);
	}
}
