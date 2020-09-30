#include "TargetBoxSelectionTool.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Editor/Core/Theme.h"

namespace Comfy::Studio::Editor
{
	TargetBoxSelectionTool::TargetBoxSelectionTool(TargetRenderWindow& renderWindow) : renderWindow(renderWindow)
	{
	}

	void TargetBoxSelectionTool::UpdateInput(Chart& chart, const TimelineTick cursorTick, const TimelineTick postHitLingerDuration)
	{
		if (Gui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
		{
			if (Gui::IsMouseClicked(SelectionButton) && !data.IsActive)
			{
				data.StartMouse = Gui::GetMousePos();
				data.StartTargetSpace = renderWindow.ScreenToTargetAreaSpace(data.StartMouse);
				data.IsActive = true;
			}
		}

		if (data.IsActive && Gui::IsMouseDown(SelectionButton))
		{
			data.EndMouse = Gui::GetMousePos();
			data.EndTargetSpace = renderWindow.ScreenToTargetAreaSpace(data.EndMouse);

			const auto& io = Gui::GetIO();
			data.Action = io.KeyShift ? ActionType::Add : io.KeyAlt ? ActionType::Remove : ActionType::Clean;

			const auto screenSpaceMin = renderWindow.TargetAreaToScreenSpace(data.StartTargetSpace);
			const auto screenSpaceMax = renderWindow.TargetAreaToScreenSpace(data.EndTargetSpace);

			constexpr f32 sizeThreshold = 4.0f;
			const f32 selectionWidth = glm::abs(screenSpaceMin.x - screenSpaceMax.x);
			const f32 selectionHeight = glm::abs(screenSpaceMin.y - screenSpaceMax.y);

			data.IsSufficientlyLarge = (selectionWidth >= sizeThreshold) || (selectionHeight >= sizeThreshold);
			if (data.IsSufficientlyLarge)
				Gui::SetActiveID(Gui::GetID(this), Gui::GetCurrentWindow());
		}

		if (Gui::IsMouseReleased(SelectionButton) && data.IsActive)
		{
			if (data.IsSufficientlyLarge)
			{
				const auto minTargetSpace = vec2(std::min(data.StartTargetSpace.x, data.EndTargetSpace.x), std::min(data.StartTargetSpace.y, data.EndTargetSpace.y));
				const auto maxTargetSpace = vec2(std::max(data.StartTargetSpace.x, data.EndTargetSpace.x), std::max(data.StartTargetSpace.y, data.EndTargetSpace.y));

				auto isTargetInSelectionRange = [&](const TimelineTarget& target) -> bool
				{
					const auto targetTick = target.Tick - TimelineTick::FromBars(1);
					const auto endTick = target.Tick + postHitLingerDuration;
					if (target.IsSelected || (cursorTick >= targetTick && cursorTick <= endTick))
					{
						const auto position = TargetPositionOrPreset(target);
						return (position.x > minTargetSpace.x && position.y >= minTargetSpace.y) && (position.x <= maxTargetSpace.x && position.y <= maxTargetSpace.y);
					}
					return false;
				};

				switch (data.Action)
				{
				case ActionType::Clean:
					for (auto& target : chart.Targets)
						target.IsSelected = isTargetInSelectionRange(target);
					break;

				case ActionType::Add:
					for (auto& target : chart.Targets)
					{
						if (isTargetInSelectionRange(target))
							target.IsSelected = true;
					}
					break;

				case ActionType::Remove:
					for (auto& target : chart.Targets)
					{
						if (isTargetInSelectionRange(target))
							target.IsSelected = false;
					}
					break;

				default:
					assert(false);
				}
			}

			data.IsActive = false;
			data.IsSufficientlyLarge = false;
		}
	}

	void TargetBoxSelectionTool::DrawSelection(ImDrawList& drawList)
	{
		if (!data.IsActive || !data.IsSufficientlyLarge)
			return;

		const auto regionMin = Gui::GetWindowPos() + vec2(1.0f);
		const auto regionMax = Gui::GetWindowPos() + Gui::GetWindowSize();

		const auto minMouse = renderWindow.TargetAreaToScreenSpace(data.StartTargetSpace);
		const auto maxMouse = renderWindow.TargetAreaToScreenSpace(data.EndTargetSpace);

		const auto start = vec2(glm::clamp(minMouse.x, regionMin.x, regionMax.x), glm::clamp(minMouse.y, regionMin.y, regionMax.y));
		const auto end = vec2(glm::clamp(maxMouse.x, regionMin.x, regionMax.x), glm::clamp(maxMouse.y, regionMin.y, regionMax.y));

		drawList.AddRectFilled(start, end, GetColor(EditorColor_TimelineSelection));
		drawList.AddRect(start, end, GetColor(EditorColor_TimelineSelectionBorder));

		if (data.Action != ActionType::Clean)
		{
			constexpr f32 circleRadius = 6.0f;
			constexpr f32 symbolSize = 2.0f;

			const auto symbolPos = start;
			const auto symbolColor = Gui::GetColorU32(ImGuiCol_Text);

			drawList.AddCircleFilled(symbolPos, circleRadius, Gui::GetColorU32(ImGuiCol_ChildBg));
			drawList.AddCircle(symbolPos, circleRadius, GetColor(EditorColor_TimelineSelectionBorder));

			if (data.Action == ActionType::Add || data.Action == ActionType::Remove)
				drawList.AddLine(symbolPos - vec2(symbolSize, 0.0f), start + vec2(symbolSize + 1.0f, 0.0f), symbolColor, 1.0f);

			if (data.Action == ActionType::Add)
				drawList.AddLine(symbolPos - vec2(0.0f, symbolSize), start + vec2(0.0f, symbolSize + 1.0f), symbolColor, 1.0f);
		}
	}
}
