#include "TargetPositionTool.h"
#include "CardinalDirection.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Editor/Chart/KeyBindings.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		// NOTE: To both prevent needless visual overload and to not overflow 16bit vertex indices too easily
		constexpr size_t MaxDistanceGuideCirclesToRender = 64;

		constexpr auto PreciseStepDistance = 1.0f;
		constexpr auto GridStepDistance = Rules::TickToDistance(TimelineTick::FromBars(1) / 16);

		constexpr vec2 SnapPositionToGrid(vec2 position)
		{
			return glm::round(position / GridStepDistance) * GridStepDistance;
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
		// TODO: What about sync placement... need full integration for horziontal/verctical pairs as well as triangle and square formations

		if (Gui::MenuItem("Flip Targets Horizontally", Input::GetKeyCodeName(KeyBindings::PositionToolFlipHorizontal), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, true);
		if (Gui::MenuItem("Flip Targets Vertically", Input::GetKeyCodeName(KeyBindings::PositionToolFlipVertical), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, false);
		if (Gui::MenuItem("Interpolate Positions", Input::GetKeyCodeName(KeyBindings::PositionToolInterpolate), false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositions(undoManager, chart);

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

		UpdateKeyboardKeyBindingsInput(chart);
		UpdateKeyboardStepInput(chart);
		UpdateMouseGrabInput(chart);
		UpdateMouseRowInput(chart);

		lastFrameSelectionCount = selectedTargetsBuffer.size();
		selectedTargetsBuffer.clear();
	}

	const char* TargetPositionTool::GetName() const
	{
		return "Position Tool";
	}

	namespace
	{
		constexpr i32 GetCircleSegmentCount(f32 radius)
		{
			// TODO: Dynamic segment count based on size
			return 64;
		}
	}

	void TargetPositionTool::DrawTickDistanceGuides(Chart& chart, ImDrawList& drawList)
	{
		if (!drawDistanceGuides)
			return;

		if (std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; }) > MaxDistanceGuideCirclesToRender)
			return;

		const auto cameraZoom = renderWindow.GetCamera().Zoom;

		// NOTE: For each pair, if any is selected within, draw distance guide around each target in previous pair
		for (size_t i = 0; i < chart.Targets.size();)
		{
			const auto& firstTargetOfPair = chart.Targets[i];

			bool anyWithinPairSelected = false;
			for (size_t pair = 0; pair < firstTargetOfPair.Flags.SyncPairCount; pair++)
			{
				if (chart.Targets[i + pair].IsSelected)
					anyWithinPairSelected = true;
			}

			if (anyWithinPairSelected && i > 0)
			{
				const auto& lastTargetOfPrevPair = chart.Targets[i - 1];
				const auto tickDistanceToPrevPair = (firstTargetOfPair.Tick - lastTargetOfPrevPair.Tick);

				auto radius = (lastTargetOfPrevPair.Flags.IsChain && !lastTargetOfPrevPair.Flags.IsChainEnd) ?
					Rules::ChainFragmentPlacementDistance : Rules::TickToDistance(tickDistanceToPrevPair);
				if (lastTargetOfPrevPair.Flags.IsChainEnd)
					radius += Rules::ChainFragmentStartEndOffsetDistance;

				const auto screenRadius = (radius * cameraZoom);

				assert(lastTargetOfPrevPair.Flags.SyncPairCount >= 1);
				for (size_t pair = 0; pair < lastTargetOfPrevPair.Flags.SyncPairCount; pair++)
				{
					const auto& prevPairTarget = chart.Targets[(i + pair - lastTargetOfPrevPair.Flags.SyncPairCount)];

					const auto screenPosition = renderWindow.TargetAreaToScreenSpace(Rules::TryGetProperties(prevPairTarget).Position);
					drawList.AddCircle(screenPosition, screenRadius, GetButtonTypeColorU32(prevPairTarget.Type, 0x84), GetCircleSegmentCount(screenRadius));
				}
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
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
		const auto dimColor = ImColor(0.1f, 0.1f, 0.1f, 0.75f);

		constexpr auto guideRadius = Rules::TickToDistance(TimelineTick::FromBars(1) / 16);
		drawList.AddCircleFilled(row.Start, guideRadius, dimColor, 32);
		drawList.AddCircle(row.Start, guideRadius, whiteColor, 32);

		drawList.AddLine(row.Start, row.Start + (vec2(+0.0f, -1.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(row.Start, row.Start + (vec2(+1.0f, +0.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(row.Start, row.Start + (vec2(+0.0f, +1.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(row.Start, row.Start + (vec2(-1.0f, +0.0f) * guideRadius), dimWhiteColor, 1.0f);

		drawList.AddCircleFilled(row.Start, 2.0f, whiteColor, 9);
		drawList.AddCircleFilled(row.Start + (direction * guideRadius), 4.0f, whiteColor, 9);
		drawList.AddLine(row.Start, row.Start + (direction * guideRadius), whiteColor, 1.0f);

		char textBuffer[32];
		const auto textView = std::string_view(textBuffer, sprintf_s(textBuffer, "[%s]", CardinalDirectionAbbreviations[static_cast<u8>(cardinal)]));
		const auto textSize = Gui::CalcTextSize(Gui::StringViewStart(textView), Gui::StringViewEnd(textView));
		const auto textPos = row.Start + vec2(-textSize.x * 0.5f, -guideRadius - textSize.y - 2.0f);

		drawList.AddRectFilled(textPos, textPos + textSize, dimColor);
		drawList.AddText(textPos, whiteColor, Gui::StringViewStart(textView), Gui::StringViewEnd(textView));
	}

	void TargetPositionTool::UpdateKeyboardKeyBindingsInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			if (Gui::IsKeyPressed(KeyBindings::PositionToolFlipHorizontal, false))
				FlipSelectedTargets(undoManager, chart, true);

			if (Gui::IsKeyPressed(KeyBindings::PositionToolFlipVertical, false))
				FlipSelectedTargets(undoManager, chart, false);

			if (Gui::IsKeyPressed(KeyBindings::PositionToolInterpolate, false))
				InterpolateSelectedTargetPositions(undoManager, chart);
		}
	}

	void TargetPositionTool::UpdateKeyboardStepInput(Chart& chart)
	{
		if (selectedTargetsBuffer.empty() || !Gui::IsWindowFocused())
			return;

		for (const auto&[key, direction] : KeyBindings::PositionToolMoveStep)
		{
			if (Gui::IsKeyPressed(key, true) && Gui::GetActiveID() == 0)
			{
				const auto stepDistance = Gui::GetIO().KeyShift ? GridStepDistance : PreciseStepDistance;
				IncrementSelectedTargetPositionsBy(undoManager, chart, direction * stepDistance);
			}
		}
	}

	void TargetPositionTool::UpdateMouseGrabInput(Chart& chart)
	{
		// TODO: Not just snap to grid but also be able to snap to other aligned targets (?) similarly to PS

		// TODO: Maybe... if all targets or a sync pair are selected (and "smartPlacement" checkbox is ticked?) auto arrange vertical sync pairs on move (?)
		//		 then toggle between smart vertical an horizontal mode using tab (indicated by menu item (?))
		//		 Then similar "smart controls" for setting button path properties, by moving the mouse to either the left or right of the vertical sync pair,
		//		 or up and down of the horizontal one. hold shift to set "sharp" / longer distances instead (?)
		// 
		//		 ~~Alternatively, the **far** easier "solution" would be to have keybindings to arrange sync pairs (?)~~ janky and unintuative

		// TODO: Check box option to distance orbit the selecte targets around each other while mouse dragging, separately from the row placement (?)

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

			constexpr auto distanceThreshold = 9.0f;
			if (glm::distance(row.Start, row.End) > distanceThreshold && (selectedTargetsBuffer.size() > 1) && row.Start != row.End && mouseWasMoved)
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
			data.NewValue.Position = (Rules::TryGetProperties(*selectedTarget).Position + positionIncrement);
		}

		undoManager.Execute<ChangeTargetListPositions>(chart, std::move(targetData));
	}

	void TargetPositionTool::ArrangeSelectedTargetsInRow(Undo::UndoManager& undoManager, Chart& chart, vec2 rowDirection, bool useStairDistance, bool backwards)
	{
		if (selectedTargetsBuffer.size() < 2)
			return;

		const auto rowAngle = glm::degrees(glm::atan(rowDirection.y, rowDirection.x));
		const auto rowCardinal = AngleToNearestCardinal(rowAngle);

		const auto horizontalCardinal =
			(rowCardinal == CardinalDirection::NorthWest || rowCardinal == CardinalDirection::SouthWest) ? CardinalDirection::West :
			(rowCardinal == CardinalDirection::NorthEast || rowCardinal == CardinalDirection::SouthEast) ? CardinalDirection::East :
			rowCardinal;

		const auto horizontalDirection = CardinalToTargetRowDirection(horizontalCardinal);

		auto getNextPos = [&](vec2 prevPosition, vec2 thisPosition, TimelineTick tickDistance, bool chain, bool chainEnd) -> vec2
		{
			auto distance = (chain && !chainEnd) ? Rules::ChainFragmentPlacementDistance :
				(useStairDistance && !chainEnd) ? Rules::TickToDistanceStair(tickDistance) : Rules::TickToDistance(tickDistance);

			if (chainEnd)
				distance += Rules::ChainFragmentStartEndOffsetDistance;

			// NOTE: Targets following one after the end of a chain need to be horizontal to avoid decimal fractions.
			//		 If a stair like pattern is desired then the corret placement would be to vertically offset the height only
			return prevPosition + ((chain ? horizontalDirection : rowDirection) * distance);
		};

		std::vector<ChangeTargetListPositionsRow::Data> targetData;
		targetData.resize(selectedTargetsBuffer.size());

		if (backwards)
		{
			targetData.back().TargetIndex = GetSelectedTargetIndex(chart, selectedTargetsBuffer.back());
			targetData.back().NewValue.Position = Rules::TryGetProperties(*selectedTargetsBuffer.back()).Position;

			for (i32 i = static_cast<i32>(selectedTargetsBuffer.size()) - 2; i >= 0; i--)
			{
				const auto& thisTarget = *selectedTargetsBuffer[i];
				const auto& nextTarget = *selectedTargetsBuffer[i + 1];
				const auto tickDistance = (nextTarget.Tick - thisTarget.Tick);

				targetData[i].TargetIndex = GetSelectedTargetIndex(chart, &thisTarget);
				targetData[i].NewValue.Position = getNextPos(targetData[i + 1].NewValue.Position, Rules::TryGetProperties(thisTarget).Position, tickDistance, thisTarget.Flags.IsChain, thisTarget.Flags.IsChainEnd);
			}
		}
		else
		{
			targetData.front().TargetIndex = GetSelectedTargetIndex(chart, selectedTargetsBuffer.front());
			targetData.front().NewValue.Position = Rules::TryGetProperties(*selectedTargetsBuffer.front()).Position;

			for (i32 i = 1; i < static_cast<i32>(selectedTargetsBuffer.size()); i++)
			{
				const auto& thisTarget = *selectedTargetsBuffer[i];
				const auto& prevTarget = *selectedTargetsBuffer[i - 1];
				const auto tickDistance = (thisTarget.Tick - prevTarget.Tick);

				targetData[i].TargetIndex = GetSelectedTargetIndex(chart, &thisTarget);
				targetData[i].NewValue.Position = getNextPos(targetData[i - 1].NewValue.Position, Rules::TryGetProperties(thisTarget).Position, tickDistance, prevTarget.Flags.IsChain, prevTarget.Flags.IsChainEnd);
			}
		}

		undoManager.Execute<ChangeTargetListPositionsRow>(chart, std::move(targetData));
	}

	void TargetPositionTool::FlipSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, bool horizontal)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		auto flipHorizontal = [](vec2 pos) -> vec2 { return vec2(Rules::PlacementAreaSize.x, pos.y) - vec2(pos.x, 0.0f); };
		auto flipVertical = [](vec2 pos) -> vec2 { return vec2(pos.x, Rules::PlacementAreaSize.y) - vec2(0.0f, pos.y); };

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			auto& target = chart.Targets[i];
			if (!target.IsSelected)
				continue;

			const auto properties = Rules::TryGetProperties(target);
			auto& data = targetData.emplace_back();
			data.TargetIndex = i;
			data.NewValue = properties;
			data.NewValue.Position = horizontal ? flipHorizontal(properties.Position) : flipVertical(properties.Position);
			data.NewValue.Angle = (horizontal) ? Rules::NormalizeAngle(-data.NewValue.Angle) : Rules::NormalizeAngle(data.NewValue.Angle - 180.0f);
			data.NewValue.Frequency *= -1.0f;
		}

		undoManager.DisallowMergeForLastCommand();
		if (horizontal)
			undoManager.Execute<FlipTargetListPropertiesHorizontal>(chart, std::move(targetData));
		else
			undoManager.Execute<FlipTargetListPropertiesVertical>(chart, std::move(targetData));
	}

	void TargetPositionTool::InterpolateSelectedTargetPositions(Undo::UndoManager& undoManager, Chart& chart)
	{
		const auto selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		const auto& lastTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [&](auto& t) { return t.IsSelected; });
		if (firstTarget.Tick == lastTarget.Tick)
			return;

		const auto startPosition = Rules::TryGetProperties(firstTarget).Position;
		const auto endPosition = Rules::TryGetProperties(lastTarget).Position;

		const auto startTick = firstTarget.Tick.Ticks();
		const auto endTick = lastTarget.Tick.Ticks();

		std::vector<InterpolateTargetListPositions::Data> targetData;
		targetData.reserve(selectionCount);

		for (i32 i = 0; i < static_cast<i32>(chart.Targets.size()); i++)
		{
			if (!chart.Targets[i].IsSelected)
				continue;

			const auto ticks = chart.Targets[i].Tick.Ticks();
			auto& data = targetData.emplace_back();
			data.TargetIndex = i;
			data.NewValue.Position = (startPosition * static_cast<f32>(endTick - ticks) + endPosition * static_cast<f32>(ticks - startTick)) / static_cast<f32>(endTick - startTick);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListPositions>(chart, std::move(targetData));
	}

	i32 TargetPositionTool::GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const
	{
		assert(chart.Targets.size() > 0);
		return static_cast<i32>(std::distance(&chart.Targets[0], selectedTarget));
	}
}
