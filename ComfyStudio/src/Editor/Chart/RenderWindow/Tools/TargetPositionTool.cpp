#include "TargetPositionTool.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		// TODO: Move to key bindings header
		static constexpr std::array MoveStepKeyMapping =
		{
			std::make_pair(Input::KeyCode_W, vec2(+0.0f, -1.0f)),
			std::make_pair(Input::KeyCode_A, vec2(-1.0f, +0.0f)),
			std::make_pair(Input::KeyCode_S, vec2(+0.0f, +1.0f)),
			std::make_pair(Input::KeyCode_D, vec2(+1.0f, +0.0f)),

			std::make_pair(Input::KeyCode_Up, vec2(+0.0f, -1.0f)),
			std::make_pair(Input::KeyCode_Left, vec2(-1.0f, +0.0f)),
			std::make_pair(Input::KeyCode_Down, vec2(+0.0f, +1.0f)),
			std::make_pair(Input::KeyCode_Right, vec2(+1.0f, +0.0f)),
		};

		constexpr auto PreciseStepDistance = 1.0f;
		constexpr auto GridStepDistance = Rules::TickToDistance(TimelineTick::FromBars(1) / 16);

		constexpr vec2 SnapPositionToGrid(vec2 position)
		{
			return glm::round(position / GridStepDistance) * GridStepDistance;
		}

		enum class CardinalDirection
		{
			North,
			East,
			South,
			West,
			NorthEast,
			SouthEast,
			SouthWest,
			NorthWest,
			Count,

			CardinalStart = North,
			CardinalEnd = West,
			IntercardinalStart = NorthEast,
			IntercardinalEnd = NorthWest,
		};

		constexpr std::array<const char*, EnumCount<CardinalDirection>()> CardinalDirectionNames =
		{
			"N", "E", "S", "W", "NE", "SE", "SW", "NW",
		};

		constexpr CardinalDirection AngleToNearestCardinal(f32 angle)
		{
			// TODO: Cleanup
			constexpr auto step = (360.0f / 8.0f) / 2.0f;

			if (angle >= -67.5 && angle <= -22.5f)
				return CardinalDirection::NorthEast;
			if (angle >= +22.5f && angle <= +67.5)
				return CardinalDirection::SouthEast;
			if (angle >= +112.5f && angle <= +157.5f)
				return CardinalDirection::SouthWest;
			if (angle >= -157.5f && angle <= -112.5f)
				return CardinalDirection::NorthWest;

			if (angle >= -35.0f && angle <= +35.0f)
				return CardinalDirection::East;
			if (angle >= -135.0f && angle <= +35.0f)
				return CardinalDirection::North;
			if ((angle >= -180.0f && angle <= -135.0f) || (angle <= 180.0f && angle >= 135.0f))
				return CardinalDirection::West;
			return CardinalDirection::South;
		}

		constexpr bool IsIntercardinal(CardinalDirection cardinal)
		{
			return (cardinal >= CardinalDirection::IntercardinalStart);
		}

		constexpr std::array<vec2, EnumCount<CardinalDirection>()> CardinalTargetRowDirections =
		{
			vec2(+0.0f, -1.0f),
			vec2(+1.0f, +0.0f),
			vec2(+0.0f, +1.0f),
			vec2(-1.0f, +0.0f),
			vec2(+Rules::PlacementStairDirection.x, -Rules::PlacementStairDirection.y),
			vec2(+Rules::PlacementStairDirection.x, +Rules::PlacementStairDirection.y),
			vec2(-Rules::PlacementStairDirection.x, +Rules::PlacementStairDirection.y),
			vec2(-Rules::PlacementStairDirection.x, -Rules::PlacementStairDirection.y),
		};

		constexpr vec2 CardinalToTargetRowDirection(CardinalDirection direction)
		{
			return CardinalTargetRowDirections[static_cast<u8>(direction)];
		}
	}

	void TargetPositionTool::OnSelected()
	{
	}

	void TargetPositionTool::OnDeselected()
	{
	}

	void TargetPositionTool::PreRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetPositionTool::PostRender(Chart& chart, Render::Renderer2D& renderer)
	{
	}

	void TargetPositionTool::OnContextMenuGUI(Chart& chart)
	{
		// TODO: Sub tools, key bindings + menu items for flipping selection horizontall / vertically (?), etc.
		Gui::MenuItem("Maybe Position Tool sub modes...", nullptr, nullptr, false);
		Gui::Separator();
	}

	void TargetPositionTool::OnOverlayGUI(Chart& chart)
	{
	}

	void TargetPositionTool::PreRenderGUI(Chart& chart, ImDrawList& drawList)
	{
	}

	void TargetPositionTool::PostRenderGUI(Chart& chart, ImDrawList& drawList)
	{
		DrawTickDistanceGuides(chart, drawList);
		DrawRowDirectionGuide(chart, drawList);
	}

	void TargetPositionTool::UpdateInput(Chart& chart)
	{
		selectedTargetsBuffer.clear();
		for (auto& target : chart.Targets)
		{
			if (target.IsSelected)
				selectedTargetsBuffer.push_back(&target);
		}

		UpdateKeyboardStepInput(chart);
		UpdateMouseGrabInput(chart);
		UpdateMouseRowInput(chart);

		selectedTargetsBuffer.clear();
	}

	const char* TargetPositionTool::GetName() const
	{
		return "Position Tool";
	}

	void TargetPositionTool::DrawTickDistanceGuides(Chart& chart, ImDrawList& drawList)
	{
		const auto zoom = renderWindow.GetCamera().Zoom;

		// TODO: Only draw if selected count < threshold to prevent overflowing 16bit vertex indices
		// TODO: Correctly handle sync pairs

		for (size_t i = 0; i < chart.Targets.size(); i++)
		{
			const auto& target = chart.Targets[i];
			const auto* prevTarget = IndexOrNull(i - 1, chart.Targets);

			if (prevTarget == nullptr || !target.IsSelected)
				continue;

			const auto tickDistance = (target.Tick - prevTarget->Tick);
			const auto distance = target.Flags.IsChain ? Rules::TickToDistanceChain(tickDistance) : Rules::TickToDistance(tickDistance);

			// TODO: Dynamic segment count based on size
			drawList.AddCircle(
				renderWindow.TargetAreaToScreenSpace(TargetPositionOrPreset(*prevTarget)),
				distance * zoom,
				GetButtonTypeColorU32(prevTarget->Type, 0x84),
				64);
		}
	}

	void TargetPositionTool::DrawRowDirectionGuide(Chart& chart, ImDrawList& drawList)
	{
		if (!row.Active)
			return;

		const auto cardinal = AngleToNearestCardinal(row.Angle);
		const auto direction = CardinalToTargetRowDirection(cardinal);

		const auto whiteColor = Gui::GetColorU32(ImGuiCol_Text);
		const auto dimWhiteColor = Gui::GetColorU32(ImGuiCol_Text, 0.35f);
		const auto dimColor = ImColor(0.1f, 0.1f, 0.1f, 0.45f);

		constexpr auto guideRadius = Rules::TickToDistance(TimelineTick::FromBars(1) / 16);
		drawList.AddCircleFilled(row.Start, guideRadius, dimColor, 32);
		drawList.AddCircle(row.Start, guideRadius, whiteColor, 32);

		drawList.AddLine(row.Start, row.Start + (vec2(+0.0f, -1.0f) * guideRadius), dimWhiteColor, 1);
		drawList.AddLine(row.Start, row.Start + (vec2(+1.0f, +0.0f) * guideRadius), dimWhiteColor, 1);
		drawList.AddLine(row.Start, row.Start + (vec2(+0.0f, +1.0f) * guideRadius), dimWhiteColor, 1);
		drawList.AddLine(row.Start, row.Start + (vec2(-1.0f, +0.0f) * guideRadius), dimWhiteColor, 1);

		drawList.AddCircleFilled(row.Start, 2.0f, whiteColor, 9);
		drawList.AddCircleFilled(row.Start + (direction * guideRadius), 4.0f, whiteColor, 9);
		drawList.AddLine(row.Start, row.Start + (direction * guideRadius), whiteColor, 1);

		char buffer[32];
		const auto bufferView = std::string_view(buffer, sprintf_s(buffer, "[%s]", CardinalDirectionNames[static_cast<u8>(cardinal)]));

		const auto textSize = Gui::CalcTextSize(Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView));
		drawList.AddText(row.Start + vec2(-textSize.x * 0.5f, -guideRadius - textSize.y - 2.0f), whiteColor, Gui::StringViewStart(bufferView), Gui::StringViewEnd(bufferView));
	}

	void TargetPositionTool::UpdateKeyboardStepInput(Chart& chart)
	{
		if (selectedTargetsBuffer.empty() || !Gui::IsWindowFocused())
			return;

		for (const auto&[key, direction] : MoveStepKeyMapping)
		{
			if (Gui::IsKeyPressed(key, true))
			{
				const auto stepDistance = Gui::GetIO().KeyShift ? GridStepDistance : PreciseStepDistance;
				IncrementSelectedTargetPositionsBy(undoManager, chart, direction * stepDistance);
			}
		}
	}

	void TargetPositionTool::UpdateMouseGrabInput(Chart& chart)
	{
		// TODO: Moving the first fragment of a chain slide should automatically move all sequential fragments as well (?)
		// TODO: Not just snap to grid but also be able to snap to other aligned targets (?) similarly to PS

		const auto mousePos = Gui::GetMousePos();

		grab.HoveredTargetIndex = -1;
		if (Gui::IsWindowHovered() && !selectedTargetsBuffer.empty())
		{
			const auto hoveredTarget = std::find_if(selectedTargetsBuffer.rbegin(), selectedTargetsBuffer.rend(), [&](auto* t)
			{
				const auto position = Rules::TryGetProperties(*t).Position;
				const auto tl = renderWindow.TargetAreaToScreenSpace(position - TargetRenderWindow::TargetHitboxSize);
				const auto br = renderWindow.TargetAreaToScreenSpace(position + TargetRenderWindow::TargetHitboxSize);

				return ImRect(tl, br).Contains(mousePos);
			});

			grab.HoveredTargetIndex = (hoveredTarget == selectedTargetsBuffer.rend()) ? -1 : GetSelectedTargetIndex(chart, *hoveredTarget);

			if (Gui::IsMouseClicked(0) && InBounds(grab.HoveredTargetIndex, chart.Targets))
			{
				grab.GrabbedTargetIndex = grab.HoveredTargetIndex;
				grab.MouseOnGrab = mousePos;
				grab.TargetPositionOnGrab = Rules::TryGetProperties(chart.Targets[grab.GrabbedTargetIndex]).Position;
				undoManager.DisallowMergeForLastCommand();
			}
		}

		if (Gui::IsMouseReleased(0))
			grab = {};

		if (Gui::IsMouseDown(0) && InBounds(grab.GrabbedTargetIndex, chart.Targets))
		{
			grab.LastPos = grab.ThisPos;
			grab.ThisPos = mousePos;

			grab.LastGridSnap = grab.ThisGridSnap;
			grab.ThisGridSnap = Gui::GetIO().KeyShift;

			Gui::SetActiveID(Gui::GetID(&grab), Gui::GetCurrentWindow());
			Gui::SetWindowFocus();
		}

		if (InBounds(grab.HoveredTargetIndex, chart.Targets) || InBounds(grab.GrabbedTargetIndex, chart.Targets))
			Gui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

		if (InBounds(grab.GrabbedTargetIndex, chart.Targets))
		{
			undoManager.ResetMergeTimeThresholdStopwatch();

			if ((grab.ThisPos != grab.LastPos) || (grab.ThisGridSnap != grab.LastGridSnap))
			{
				auto grabbedMovedPosition = glm::round(grab.TargetPositionOnGrab + ((grab.ThisPos - grab.MouseOnGrab) / renderWindow.GetCamera().Zoom));
				if (Gui::GetIO().KeyShift)
					grabbedMovedPosition = SnapPositionToGrid(grabbedMovedPosition);

				const auto increment = (grabbedMovedPosition - Rules::TryGetProperties(chart.Targets[grab.GrabbedTargetIndex]).Position);
				if (increment != vec2(0.0f))
					IncrementSelectedTargetPositionsBy(undoManager, chart, increment);
			}
		}
	}

	void TargetPositionTool::UpdateMouseRowInput(Chart& chart)
	{
		if (!selectedTargetsBuffer.empty() && Gui::IsWindowHovered() && Gui::IsWindowFocused())
		{
			if (Gui::IsMouseClicked(0))
			{
				row.Start = Gui::GetMousePos();
				row.Active = true;
				row.Backwards = Gui::GetIO().KeyAlt;
				undoManager.DisallowMergeForLastCommand();
			}
		}

		if (Gui::IsMouseReleased(0))
			row = {};

		if (row.Active)
		{
			Gui::SetActiveID(Gui::GetID(&row), Gui::GetCurrentWindow());
			Gui::SetMouseCursor(ImGuiMouseCursor_Hand);

			row.End = Gui::GetMousePos();

			row.Direction = glm::normalize(row.End - row.Start);
			row.Angle = glm::degrees(glm::atan(row.Direction.y, row.Direction.x));

			const auto cardinal = AngleToNearestCardinal(row.Angle);
			const auto rowDirection = CardinalToTargetRowDirection(cardinal);

			const bool mouseWasMoved = (Gui::GetIO().MouseDelta.x != 0.0f || Gui::GetIO().MouseDelta.y != 0.0f);
			undoManager.ResetMergeTimeThresholdStopwatch();

			if (glm::distance(row.Start, row.End) > 3.0f && (selectedTargetsBuffer.size() > 1) && row.Start != row.End && mouseWasMoved)
				ArrangeSelectedTargetsInRow(undoManager, chart, rowDirection, IsIntercardinal(cardinal), row.Backwards);
		}
	}

	void TargetPositionTool::IncrementSelectedTargetPositionsBy(Undo::UndoManager& undoManager, Chart& chart, vec2 positionIncrement)
	{
		if (selectedTargetsBuffer.empty())
			return;

		std::vector<ChangeTargetListPositions::Data> targetData;
		targetData.reserve(selectedTargetsBuffer.size());

		for (auto* selectedTarget : selectedTargetsBuffer)
		{
			auto& data = targetData.emplace_back();
			data.TargetIndex = GetSelectedTargetIndex(chart, selectedTarget);
			data.NewValue.Position = glm::round(Rules::TryGetProperties(*selectedTarget).Position + positionIncrement);
		}

		undoManager.Execute<ChangeTargetListPositions>(chart, std::move(targetData));
	}

	void TargetPositionTool::ArrangeSelectedTargetsInRow(Undo::UndoManager& undoManager, Chart& chart, vec2 rowDirection, bool useStairDistance, bool backwards)
	{
		if (selectedTargetsBuffer.size() < 2)
			return;

		auto getNextPos = [&](vec2 prevPosition, vec2 thisPosition, TimelineTick tickDistance) -> vec2
		{
			if (useStairDistance)
				return (prevPosition + (rowDirection * Rules::TickToDistanceStair(tickDistance)));
			else
				return (prevPosition + (rowDirection * Rules::TickToDistance(tickDistance)));
		};

		std::vector<ChangeTargetListPositionsRow::Data> targetData;
		targetData.resize(selectedTargetsBuffer.size());

		if (backwards)
		{
			targetData.back().TargetIndex = GetSelectedTargetIndex(chart, selectedTargetsBuffer.back());
			targetData.back().NewValue.Position = Rules::TryGetProperties(*selectedTargetsBuffer.back()).Position;

			for (i32 i = static_cast<i32>(selectedTargetsBuffer.size()) - 2; i >= 0; i--)
			{
				const auto tickDistance = (selectedTargetsBuffer[i + 1]->Tick - selectedTargetsBuffer[i]->Tick);
				targetData[i].TargetIndex = GetSelectedTargetIndex(chart, selectedTargetsBuffer[i]);
				targetData[i].NewValue.Position = getNextPos(targetData[i + 1].NewValue.Position, Rules::TryGetProperties(*selectedTargetsBuffer[i]).Position, tickDistance);
			}
		}
		else
		{
			targetData.front().TargetIndex = GetSelectedTargetIndex(chart, selectedTargetsBuffer.front());
			targetData.front().NewValue.Position = Rules::TryGetProperties(*selectedTargetsBuffer.front()).Position;

			for (i32 i = 1; i < static_cast<i32>(selectedTargetsBuffer.size()); i++)
			{
				const auto tickDistance = (selectedTargetsBuffer[i]->Tick - selectedTargetsBuffer[i - 1]->Tick);
				targetData[i].TargetIndex = GetSelectedTargetIndex(chart, selectedTargetsBuffer[i]);
				targetData[i].NewValue.Position = getNextPos(targetData[i - 1].NewValue.Position, Rules::TryGetProperties(*selectedTargetsBuffer[i]).Position, tickDistance);
			}
		}

		undoManager.Execute<ChangeTargetListPositionsRow>(chart, std::move(targetData));
	}

	i32 TargetPositionTool::GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const
	{
		assert(chart.Targets.size() > 0);
		return static_cast<i32>(std::distance(&chart.Targets[0], selectedTarget));
	}
}
