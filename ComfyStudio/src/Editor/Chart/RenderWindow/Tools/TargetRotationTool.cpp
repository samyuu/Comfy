#include "TargetRotationTool.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Editor/Chart/KeyBindings.h"

namespace Comfy::Studio::Editor
{
	void TargetRotationTool::OnSelected()
	{
	}

	void TargetRotationTool::OnDeselected()
	{
	}

	void TargetRotationTool::PreRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetRotationTool::PostRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetRotationTool::OnContextMenuGUI(Chart& chart)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });

		if (Gui::MenuItem("Invert Target Frequencies", Input::GetKeyCodeName(KeyBindings::RotationToolInvertFrequencies), false, (selectionCount > 0)))
			InvertSelectedTargetFrequencies(undoManager, chart);
		if (Gui::MenuItem("Interpolate Angles Clockwise", Input::GetKeyCodeName(KeyBindings::RotationToolInterpolateClockwise), false, (selectionCount > 0)))
			InterpolateSelectedTargetAngles(undoManager, chart, true);
		if (Gui::MenuItem("Interpolate Angles Counterclockwise", Input::GetKeyCodeName(KeyBindings::RotationToolInterpolateCounterclockwise), false, (selectionCount > 0)))
			InterpolateSelectedTargetAngles(undoManager, chart, false);
	}

	void TargetRotationTool::OnOverlayGUI(Chart& chart)
	{
	}

	void TargetRotationTool::PreRenderGUI(Chart& chart, ImDrawList& drawList)
	{
	}

	void TargetRotationTool::PostRenderGUI(Chart& chart, ImDrawList& drawList)
	{
		DrawTargetAngleGuides(chart, drawList);
		DrawAngleDragGuide(chart, drawList);
	}

	void TargetRotationTool::UpdateInput(Chart& chart)
	{
		// TODO: Move into separate functions
		if (Gui::IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			if (Gui::IsKeyPressed(KeyBindings::RotationToolInvertFrequencies, false))
				InvertSelectedTargetFrequencies(undoManager, chart);
			if (Gui::IsKeyPressed(KeyBindings::RotationToolInterpolateClockwise, false))
				InterpolateSelectedTargetAngles(undoManager, chart, true);
			if (Gui::IsKeyPressed(KeyBindings::RotationToolInterpolateCounterclockwise, false))
				InterpolateSelectedTargetAngles(undoManager, chart, false);
		}
		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			if (const auto wheel = Gui::GetIO().MouseWheel; wheel != 0.0f)
				IncrementSelectedTargetAnglesBy(undoManager, chart, -wheel);
		}

		UpdateMouseAngleDragInput(chart);
	}

	const char* TargetRotationTool::GetName() const
	{
		return "Rotation Tool";
	}

	void TargetRotationTool::DrawTargetAngleGuides(Chart& chart, ImDrawList& drawList)
	{
		constexpr size_t maxPathsToDraw = 64;
		size_t pathDrawCount = 0;

		// TODO: Is this a good solution..?
		if (angleDrag.Active)
		{
			for (auto& target : chart.Targets)
			{
				if (!target.IsSelected)
					continue;

				DrawTargetButtonAngleLine(drawList, Rules::TryGetProperties(target), GetButtonTypeColorU32(target.Type, 0xD6), 2.0f);
				if (pathDrawCount++ >= maxPathsToDraw)
					break;
			}
		}
		else
		{
			for (auto& target : chart.Targets)
			{
				if (!target.IsSelected)
					continue;

				DrawTargetButtonPathCurve(drawList, Rules::TryGetProperties(target), GetButtonTypeColorU32(target.Type, 0xD6), 2.0f);
				if (pathDrawCount++ >= maxPathsToDraw)
					break;
			}
		}


	}

	void TargetRotationTool::DrawTargetButtonAngleLine(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const
	{
		const auto angleRadians = glm::radians(properties.Angle) - glm::radians(90.0f);
		const auto angleVector = vec2(glm::cos(angleRadians), glm::sin(angleRadians));

		const auto start = renderWindow.TargetAreaToScreenSpace(properties.Position);
		const auto end = renderWindow.TargetAreaToScreenSpace(properties.Position + (angleVector * properties.Distance));

		drawList.AddLine(start, end, color, thickness);
	}

	void TargetRotationTool::DrawTargetButtonPathCurve(ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness) const
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

	void TargetRotationTool::DrawAngleDragGuide(Chart& chart, ImDrawList& drawList)
	{
		if (!angleDrag.Active)
			return;

		const auto whiteColor = Gui::GetColorU32(ImGuiCol_Text);
		const auto dimWhiteColor = Gui::GetColorU32(ImGuiCol_Text, 0.35f);
		const auto dimColor = ImColor(0.1f, 0.1f, 0.1f, 0.75f);

		constexpr auto guideRadius = Rules::TickToDistance(TimelineTick::FromBars(1) / 16);
		drawList.AddCircleFilled(angleDrag.Start, guideRadius, dimColor, 32);
		drawList.AddCircle(angleDrag.Start, guideRadius, whiteColor, 32);

		drawList.AddLine(angleDrag.Start, angleDrag.Start + (vec2(+0.0f, -1.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(angleDrag.Start, angleDrag.Start + (vec2(+1.0f, +0.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(angleDrag.Start, angleDrag.Start + (vec2(+0.0f, +1.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(angleDrag.Start, angleDrag.Start + (vec2(-1.0f, +0.0f) * guideRadius), dimWhiteColor, 1.0f);

		const auto directionRadians = glm::radians(angleDrag.DegreesTargetAngle - 90.0f);
		const auto direction = vec2(glm::cos(directionRadians), glm::sin(directionRadians));

		drawList.AddCircleFilled(angleDrag.Start, 2.0f, whiteColor, 9);
		drawList.AddCircleFilled(angleDrag.Start + (direction * guideRadius), 4.0f, whiteColor, 9);
		drawList.AddLine(angleDrag.Start, angleDrag.Start + (direction * guideRadius), whiteColor, 1);

		char buffer[32];
		const auto bufferView = std::string_view(buffer, sprintf_s(buffer, (angleDrag.Start != angleDrag.End) ? "[%.1f deg]" : "[--.- deg]", angleDrag.DegreesTargetAngle));

		const auto textSize = Gui::CalcTextSize(Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView));
		drawList.AddText(angleDrag.Start + vec2(-textSize.x * 0.5f, -guideRadius - textSize.y - 2.0f), whiteColor, Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView));
	}

	void TargetRotationTool::UpdateMouseAngleDragInput(Chart& chart)
	{
		if (angleDrag.Active && Gui::IsMouseReleased(0))
			undoManager.DisallowMergeForLastCommand();

		if (!Gui::IsWindowFocused() || Gui::IsMouseReleased(0))
			angleDrag = {};

		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			if (Gui::IsMouseClicked(0) && !Gui::IsAnyItemHovered())
			{
				if (std::any_of(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) {return t.IsSelected; }))
				{
					angleDrag.Active = true;
					angleDrag.Start = Gui::GetMousePos();
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

			angleDrag.End = Gui::GetMousePos();
			angleDrag.Direction = glm::normalize(angleDrag.End - angleDrag.Start);
			angleDrag.DegreesTargetAngle = glm::degrees(glm::atan(angleDrag.Direction.y, angleDrag.Direction.x)) + 90.0f;

			constexpr f32 roughAngleSnap = 15.0f, preciseAngleSnap = 0.1f, angleSnap = 0.1f;

			const auto snap = angleDrag.RoughStep ? roughAngleSnap : angleDrag.PreciseStep ? preciseAngleSnap : angleSnap;
			angleDrag.DegreesTargetAngle = Rules::NormalizeAngle(glm::round(angleDrag.DegreesTargetAngle / snap) * snap);

			constexpr auto distanceThreshold = 4.0f;
			if (glm::distance(angleDrag.Start, angleDrag.End) > distanceThreshold)
				SetSelectedTargetAnglesTo(undoManager, chart, angleDrag.DegreesTargetAngle);
		}
	}

	void TargetRotationTool::IncrementSelectedTargetAnglesBy(Undo::UndoManager& undoManager, Chart& chart, f32 increment)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
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
				data.NewValue.Angle = Rules::TryGetProperties(target).Angle + increment;
			}
		}

		undoManager.Execute<ChangeTargetListAngles>(chart, std::move(targetData));
	}

	void TargetRotationTool::SetSelectedTargetAnglesTo(Undo::UndoManager& undoManager, Chart& chart, f32 newAngle)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
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

	void TargetRotationTool::InvertSelectedTargetFrequencies(Undo::UndoManager& undoManager, Chart& chart)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
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

	void TargetRotationTool::InterpolateSelectedTargetAngles(Undo::UndoManager& undoManager, Chart& chart, bool clockwise)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		const auto& lastTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [&](auto& t) { return t.IsSelected; });

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
}
