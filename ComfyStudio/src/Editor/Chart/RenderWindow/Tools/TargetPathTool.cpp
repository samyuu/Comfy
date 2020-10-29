#include "TargetPathTool.h"
#include "CardinalDirection.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Editor/Chart/KeyBindings.h"

// NOTE: Explicit code point macro because git was complaining about encoding issues
//		 U+00B0: Degree Sign
//		 U+00BA: Masculine Ordinal (easier to read with the current font)
// #define DEGREE_SIGN "\xC2\xB0"
#define DEGREE_SIGN "\xC2\xBA"

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
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });

		if (Gui::MenuItem("Invert Target Frequencies", Input::GetKeyCodeName(KeyBindings::PathToolInvertFrequencies), false, (selectionCount > 0)))
			InvertSelectedTargetFrequencies(undoManager, chart);
		if (Gui::MenuItem("Interpolate Angles Clockwise", Input::GetKeyCodeName(KeyBindings::PathToolInterpolateClockwise), false, (selectionCount > 0)))
			InterpolateSelectedTargetAngles(undoManager, chart, true);
		if (Gui::MenuItem("Interpolate Angles Counterclockwise", Input::GetKeyCodeName(KeyBindings::PathToolInterpolateCounterclockwise), false, (selectionCount > 0)))
			InterpolateSelectedTargetAngles(undoManager, chart, false);

		if (Gui::BeginMenu("Angle Variation Settings"))
		{
			constexpr auto formatString = ("%.2f" DEGREE_SIGN);
			Gui::InputFloat("Increment Per Beat", &angleVariation.IncrementPerBeat, 1.0f, 5.0f, formatString);
			Gui::InputFloat("Increment Per Beat (Slope)", &angleVariation.IncrementPerBeatSlope, 1.0f, 5.0f, formatString);

			constexpr auto defaultVariation = AngleVariationData {};
			const bool isDefault = (angleVariation.IncrementPerBeat == defaultVariation.IncrementPerBeat && angleVariation.IncrementPerBeatSlope == defaultVariation.IncrementPerBeatSlope);

			Gui::PushItemDisabledAndTextColorIf(isDefault);
			if (Gui::Button("Set Default", vec2(Gui::GetContentRegionAvailWidth() * 0.645f, 0.0f)))
				angleVariation = defaultVariation;
			Gui::PopItemDisabledAndTextColorIf(isDefault);

			Gui::EndMenu();
		}

		if (Gui::MenuItem("Apply Angle Variations Positive", Input::GetKeyCodeName(KeyBindings::PathToolApplyAngleVariationsPositive), false, (selectionCount > 0)))
			ApplySelectedTargetAngleVariations(undoManager, chart, +1.0f);
		if (Gui::MenuItem("Apply Angle Variations Negative", Input::GetKeyCodeName(KeyBindings::PathToolApplyAngleVariationsNegative), false, (selectionCount > 0)))
			ApplySelectedTargetAngleVariations(undoManager, chart, -1.0f);
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
		constexpr size_t maxPathsToDraw = 64;
		size_t pathDrawCount = 0;

		if (angleDrag.Active || angleScroll.Active)
		{
			const auto* dragTarget = IndexOrNull(angleDrag.Active ? angleDrag.TargetIndex : angleScroll.TargetIndex, chart.Targets);

			for (auto& target : chart.Targets)
			{
				if (target.IsSelected && &target != dragTarget)
				{
					DrawTargetButtonAngleLine(drawList, Rules::TryGetProperties(target), GetButtonTypeColorU32(target.Type, 0x76), 2.0f);
					if (pathDrawCount++ >= maxPathsToDraw)
						break;
				}
			}

			// NOTE: For readibility sake always draw the main drag arrow on top
			if (dragTarget != nullptr)
				DrawTargetButtonAngleArrowLine(drawList, Rules::TryGetProperties(*dragTarget), GetButtonTypeColorU32(dragTarget->Type, 0xD6), 2.0f);
		}
		else
		{
			for (auto& target : chart.Targets)
			{
				if (target.IsSelected)
				{
					DrawTargetButtonPathCurve(drawList, Rules::TryGetProperties(target), GetButtonTypeColorU32(target.Type, 0xD6), 2.0f);
					if (pathDrawCount++ >= maxPathsToDraw)
						break;
				}
			}
		}
	}

	void TargetPathTool::DrawTargetButtonAngleLine(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const
	{
		const auto angleRadians = glm::radians(properties.Angle) - glm::radians(90.0f);
		const auto angleDirection = vec2(glm::cos(angleRadians), glm::sin(angleRadians));

		const auto start = renderWindow.TargetAreaToScreenSpace(properties.Position);
		const auto end = renderWindow.TargetAreaToScreenSpace(properties.Position + (angleDirection * properties.Distance));

		drawList.AddLine(start, end, color, thickness);
	}

	void TargetPathTool::DrawTargetButtonAngleArrowLine(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const
	{
		static constexpr struct ArrowSettings
		{
			f32 HeadSpacing = 96.0f;
			f32 HeadEndSpacingFactor = 0.6f;
			f32 HeadSize = 168.0f;
			f32 HeadAngle = 16.0f;
			vec4 BackgroundColor = vec4(0.16f, 0.16f, 0.16f, 0.95f);
		} settings;

		const auto angleRadians = glm::radians(properties.Angle) - glm::radians(90.0f);
		const auto angleDirection = vec2(glm::cos(angleRadians), glm::sin(angleRadians));

		const auto start = renderWindow.TargetAreaToScreenSpace(properties.Position);
		const auto end = renderWindow.TargetAreaToScreenSpace(properties.Position + (angleDirection * properties.Distance));

		const auto headSpacing = settings.HeadSpacing * renderWindow.GetCamera().Zoom;
		const auto headSize = (settings.HeadSize * renderWindow.GetCamera().Zoom) / 2.0f;
		const auto headRadians = glm::radians(settings.HeadAngle);

		const auto headRadiansL = (angleRadians - headRadians);
		const auto headRadiansR = (angleRadians + headRadians);
		const auto headDirectionL = vec2(glm::cos(headRadiansL), glm::sin(headRadiansL));
		const auto headDirectionR = vec2(glm::cos(headRadiansR), glm::sin(headRadiansR));
		const auto backgroundColor = Gui::ColorConvertFloat4ToU32(settings.BackgroundColor * Gui::ColorConvertU32ToFloat4(color));

		const auto startEndDistance = (properties.Distance <= 0.0f) ? 0.0f : glm::distance(start, end);
		const auto headCount = std::max(1, static_cast<i32>(glm::floor(startEndDistance / headSpacing)));

		for (i32 head = 0; head < headCount; head++)
		{
			const auto headDistance = ((head + 0) * headSpacing) + (headSpacing * settings.HeadEndSpacingFactor) + (thickness / 2.0f);
			const auto headDistanceNext = ((head + 1) == headCount) ? startEndDistance : glm::min(startEndDistance, ((head + 1) * headSpacing));

			const auto headStart = start + (headDistance * angleDirection);
			const auto headEnd = start + (headDistanceNext * angleDirection);

			drawList.AddLine(headStart, headEnd, color, thickness);
		}

		for (i32 head = 0; head < headCount; head++)
		{
			const auto headDistance = (head * headSpacing);

			const auto headStart = start + (headDistance * angleDirection);
			const auto headEnd = start + ((headDistance + (headSpacing * settings.HeadEndSpacingFactor)) * angleDirection);

			const auto headEndL = headStart + (headDirectionL * headSize);
			const auto headEndR = headStart + (headDirectionR * headSize);

			// NOTE: Primarily to hide the AA triangle seams at some angles
			drawList.AddLine(headStart, headEnd, backgroundColor, 1.0f);
			drawList.AddTriangleFilled(headStart, headEndL, headEnd, backgroundColor);
			drawList.AddTriangleFilled(headEnd, headEndR, headStart, backgroundColor);

			drawList.AddLine(headStart, headEndL, color, thickness);
			drawList.AddLine(headStart, headEndR, color, thickness);
			drawList.AddLine(headEndL, headEnd, color, thickness);
			drawList.AddLine(headEndR, headEnd, color, thickness);
		}
	}

	void TargetPathTool::DrawTargetButtonPathCurve(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const
	{
		if (properties.Frequency == 0.0f || properties.Amplitude == 0.0f)
		{
			DrawTargetButtonAngleLine(drawList, properties, color, thickness);
		}
		else
		{
			constexpr f32 step = (1.0f / 32.0f);

			drawList.PathLineTo(renderWindow.TargetAreaToScreenSpace(GetButtonPathSinePoint(0.0f, properties)));
			for (f32 i = step; i <= 1.0f; i += step)
				drawList.PathLineTo(renderWindow.TargetAreaToScreenSpace(GetButtonPathSinePoint(i, properties)));
			drawList.PathStroke(color, false, thickness);
		}
	}

	void TargetPathTool::DrawAngleDragGuide(Chart& chart, ImDrawList& drawList)
	{
		if (!angleDrag.Active && !angleScroll.Active)
			return;

		const auto dragTarget = IndexOrNull(angleDrag.Active ? angleDrag.TargetIndex : angleScroll.TargetIndex, chart.Targets);
		if (dragTarget == nullptr)
			return;

		const auto targetProperties = Rules::TryGetProperties(*dragTarget);
		const auto screenPosition = renderWindow.TargetAreaToScreenSpace(targetProperties.Position);
		const auto cameraZoom = renderWindow.GetCamera().Zoom;

		const auto buttonTypeColor = GetButtonTypeColorU32(dragTarget->Type, 0x80);

		drawList.AddLine(
			screenPosition,
			screenPosition + vec2(0.0f, -Rules::TickToDistance(TimelineTick::FromBars(1) / 6) * cameraZoom),
			buttonTypeColor,
			2.0f);

		const auto radius = Rules::TickToDistance(TimelineTick::FromBars(1) / 8) * cameraZoom;
		const auto angle = Rules::NormalizeAngle(targetProperties.Angle) - 90.0f;

		drawList.PathArcTo(screenPosition, radius - 0.5f, glm::radians(-90.0f), glm::radians(angle), 32);
		drawList.PathStroke(buttonTypeColor, false, 2.0f);

		char buffer[64];
		const auto bufferView = std::string_view(buffer, sprintf_s(buffer, ("%.2f" DEGREE_SIGN), targetProperties.Angle));

		const auto textPadding = vec2(3.0f, 1.0f);
		const auto textSize = Gui::CalcTextSize(Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView)) + textPadding;
		const auto textPos = renderWindow.TargetAreaToScreenSpace(
			targetProperties.Position + vec2(Rules::TickToDistance(TimelineTick::FromBars(1) / 10)) * vec2(1.0f, -1.0f)) - (textSize / 2.0f);

		const auto dimColor = ImColor(0.1f, 0.1f, 0.1f, 0.85f);
		drawList.AddRectFilled(textPos, textPos + textSize, dimColor);
		drawList.AddText(textPos + (textPadding / 2.0f), Gui::GetColorU32(ImGuiCol_Text), Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView));
	}

	void TargetPathTool::UpdateKeyboardKeyBindingsInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			if (Gui::IsKeyPressed(KeyBindings::PathToolInvertFrequencies, false))
				InvertSelectedTargetFrequencies(undoManager, chart);
			if (Gui::IsKeyPressed(KeyBindings::PathToolInterpolateClockwise, false))
				InterpolateSelectedTargetAngles(undoManager, chart, true);
			if (Gui::IsKeyPressed(KeyBindings::PathToolInterpolateCounterclockwise, false))
				InterpolateSelectedTargetAngles(undoManager, chart, false);
			if (Gui::IsKeyPressed(KeyBindings::PathToolApplyAngleVariationsPositive, false))
				ApplySelectedTargetAngleVariations(undoManager, chart, +1.0f);
			if (Gui::IsKeyPressed(KeyBindings::PathToolApplyAngleVariationsNegative, false))
				ApplySelectedTargetAngleVariations(undoManager, chart, -1.0f);
		}
	}

	void TargetPathTool::UpdateMouseAngleScrollInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			if (const auto& io = Gui::GetIO(); io.MouseWheel != 0.0f)
			{
				constexpr f32 roughAngleStep = 5.0f, preciseAngleStep = 0.1f, angleStep = 1.0f;
				constexpr f32 scrollDirection = -1.0f;

				const auto angleIncrement = io.KeyShift ? roughAngleStep : io.KeyAlt ? preciseAngleStep : angleStep;
				IncrementSelectedTargetAnglesBy(undoManager, chart, (angleIncrement * io.MouseWheel * scrollDirection));

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

				const auto targetIndex = angleDrag.UseLastTarget ?
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
				constexpr f32 roughAngleSnap = 15.0f, preciseAngleSnap = 0.1f, angleSnap = 1.0f;

				const auto snap = angleDrag.RoughStep ? roughAngleSnap : angleDrag.PreciseStep ? preciseAngleSnap : angleSnap;
				angleDrag.DegreesTargetAngle = Rules::NormalizeAngle(glm::round(angleDrag.DegreesTargetAngle / snap) * snap);

				constexpr auto distanceThreshold = 4.0f;
				if (glm::distance(angleDrag.StartTarget, angleDrag.EndMouse) > distanceThreshold)
					SetSelectedTargetAnglesTo(undoManager, chart, angleDrag.DegreesTargetAngle);
			}
		}
	}

	void TargetPathTool::IncrementSelectedTargetAnglesBy(Undo::UndoManager& undoManager, Chart& chart, f32 increment)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			if (const auto& target = chart.Targets[i]; target.IsSelected)
			{
				auto& data = targetData.emplace_back();
				data.TargetIndex = i;
				data.NewValue.Angle = Rules::NormalizeAngle(Rules::TryGetProperties(target).Angle + increment);
			}
		}

		undoManager.Execute<ChangeTargetListAngles>(chart, std::move(targetData));
	}

	void TargetPathTool::SetSelectedTargetAnglesTo(Undo::UndoManager& undoManager, Chart& chart, f32 newAngle)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			if (const auto& target = chart.Targets[i]; target.IsSelected)
			{
				auto& data = targetData.emplace_back();
				data.TargetIndex = i;
				data.NewValue.Angle = newAngle;
			}
		}

		undoManager.Execute<ChangeTargetListAngles>(chart, std::move(targetData));
	}

	void TargetPathTool::InvertSelectedTargetFrequencies(Undo::UndoManager& undoManager, Chart& chart)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			auto& target = chart.Targets[i];
			if (!target.IsSelected)
				continue;

			const auto targetFrequency = Rules::TryGetProperties(target).Frequency;

			auto& data = targetData.emplace_back();
			data.TargetIndex = i;
			data.NewValue.Frequency = (targetFrequency == 0.0f) ? 0.0f : (targetFrequency * -1.0f);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InvertTargetListFrequencies>(chart, std::move(targetData));
	}

	void TargetPathTool::InterpolateSelectedTargetAngles(Undo::UndoManager& undoManager, Chart& chart, bool clockwise)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		const auto& lastTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [](auto& t) { return t.IsSelected; });

		// BUG: Negative -> position angles not handled correctly (?)
		const auto startAngle = Rules::TryGetProperties(firstTarget).Angle;
		const auto endAngle = clockwise ? Rules::TryGetProperties(lastTarget).Angle : (Rules::TryGetProperties(lastTarget).Angle - 360.0f);

		const auto startTick = firstTarget.Tick.Ticks();
		const auto endTick = lastTarget.Tick.Ticks();

		std::vector<InterpolateTargetListAngles::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			if (!chart.Targets[i].IsSelected)
				continue;

			const auto ticks = chart.Targets[i].Tick.Ticks();
			auto& data = targetData.emplace_back();
			data.TargetIndex = i;
			data.NewValue.Angle = Rules::NormalizeAngle(((startAngle * static_cast<f32>(endTick - ticks) + endAngle * static_cast<f32>(ticks - startTick)) / static_cast<f32>(endTick - startTick)));
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListAngles>(chart, std::move(targetData));
	}

	void TargetPathTool::ApplySelectedTargetAngleVariations(Undo::UndoManager& undoManager, Chart& chart, f32 direction)
	{
		assert(std::isnormal(direction));

		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<ChangeTargetListAngles::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			if (!chart.Targets[i].IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.TargetIndex = i;
			data.NewValue = Rules::TryGetProperties(chart.Targets[i]);
		}

		for (size_t i = 1; i < targetData.size(); i++)
		{
			auto& prevData = targetData[i - 1];
			auto& thisData = targetData[i];

			const auto& prevTarget = chart.Targets[prevData.TargetIndex];
			const auto& thisTarget = chart.Targets[thisData.TargetIndex];

			const auto tickDifference = (prevTarget.Tick - thisTarget.Tick);

			const auto directionToLastTarget = glm::normalize(prevData.NewValue.Position - thisData.NewValue.Position);
			const auto angleToLastTarget = glm::degrees(glm::atan(directionToLastTarget.y, directionToLastTarget.x));
			const bool isSlope = IsIntercardinal(AngleToNearestCardinal(angleToLastTarget));

			const auto incrementPerBeat = (isSlope ? angleVariation.IncrementPerBeatSlope : angleVariation.IncrementPerBeat);
			const auto angleIncrement = (tickDifference.BeatsFraction() * incrementPerBeat * direction);

			thisData.NewValue.Angle = Rules::NormalizeAngle(prevData.NewValue.Angle + angleIncrement);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<ApplyTargetListAngleVarations>(chart, std::move(targetData));
	}
}
