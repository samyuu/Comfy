#include "TargetPositionTool.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Editor/Chart/KeyBindings.h"
#include "Core/ComfyStudioSettings.h"
#include <numeric>

namespace Comfy::Studio::Editor
{
	namespace
	{
		// NOTE: To both prevent needless visual overload and to not overflow 16bit vertex indices too easily
		constexpr size_t MaxDistanceGuideCirclesToRender = 64;

		constexpr f32 PreciseStepDistance = 1.0f;
		constexpr f32 GridStepDistance = Rules::TickToDistance(BeatTick::FromBars(1) / 16);

		constexpr vec2 SnapPositionTo(vec2 position, f32 snapDistance)
		{
			return glm::round(position / snapDistance) * snapDistance;
		}

		// TODO: Move to Rules:: namespace (?)
		constexpr vec2 SnapPositionToGrid(vec2 position)
		{
			return SnapPositionTo(position, GridStepDistance);
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
		static_assert(KeyBindings::PositionToolFlipHorizontal == Input::KeyCode_H);
		static_assert(KeyBindings::PositionToolFlipVertical == Input::KeyCode_J);

		if (Gui::MenuItem("Flip Targets Horizontally", Input::GetKeyCodeName(KeyBindings::PositionToolFlipHorizontal), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::Horizontal);
		if (Gui::MenuItem("Flip Targets Horizontally (Local)", "Alt + H", false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::HorizontalLocal);

		if (Gui::MenuItem("Flip Targets Vertically", Input::GetKeyCodeName(KeyBindings::PositionToolFlipVertical), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::Vertical);
		if (Gui::MenuItem("Flip Targets Vertically (Local)", "Alt + J", false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::VerticalLocal);

		Gui::Separator();

		// TODO: Implement keybinding string conversion with support for modifiers
		static_assert(KeyBindings::PositionToolPositionInRow == Input::KeyCode_U);

		if (Gui::MenuItem("Position in Row", Input::GetKeyCodeName(KeyBindings::PositionToolPositionInRow), false, (lastFrameSelectionCount > 0)))
			PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, false);
		if (Gui::MenuItem("Position in Row (Back)", "Alt + U", false, (lastFrameSelectionCount > 0)))
			PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, true);
		Gui::Separator();

		if (Gui::MenuItem("Interpolate Positions Linear", Input::GetKeyCodeName(KeyBindings::PositionToolInterpolateLinear), false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositionsLinear(undoManager, chart);

		static_assert(KeyBindings::PositionToolInterpolateCircular == Input::KeyCode_O);

		if (Gui::MenuItem("Interpolate Positions Circular", Input::GetKeyCodeName(KeyBindings::PositionToolInterpolateCircular), false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositionsCircular(undoManager, chart, +1.0f);
		if (Gui::MenuItem("Interpolate Positions Circular (Flip)", "Alt + O", false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositionsCircular(undoManager, chart, -1.0f);
		Gui::Separator();

		if (Gui::MenuItem("Stack Targets", Input::GetKeyCodeName(KeyBindings::PositionToolStackPositions), false, (lastFrameSelectionCount > 0)))
			StackSelectedTargetPositions(undoManager, chart);

		if (Gui::BeginMenu("Snap Positions To", (lastFrameSelectionCount > 0)))
		{
			if (Gui::MenuItem("Nearest Whole", "(1 px)"))
				SnapSelectedTargetPositions(undoManager, chart, 1.0f);

			static_assert(GridStepDistance == 48.0f);
			if (Gui::MenuItem("Nearest Grid", "(48 px)"))
				SnapSelectedTargetPositions(undoManager, chart, GridStepDistance);

			for (const i32 division : std::array { 24, 32, 48, 64, 96, 192 })
			{
				const f32 snapDistance = Rules::TickToDistance(BeatTick::FromBars(1) / division);
				char labelBuffer[16], shortcutBuffer[8];

				sprintf_s(labelBuffer, "Nearest 1 / %d", division);
				sprintf_s(shortcutBuffer, "(%.f px)", snapDistance);

				if (Gui::MenuItem(labelBuffer, shortcutBuffer))
					SnapSelectedTargetPositions(undoManager, chart, snapDistance);
			}

			Gui::EndMenu();
		}

#if COMFY_DEBUG && 0 // TODO: Two step rotation tool
		Gui::MenuItem("Rotate Targets...", "?", false, false);
#endif

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

		const f32 cameraZoom = renderWindow.GetCamera().Zoom;

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

				const f32 screenRadius = (radius * cameraZoom);

				assert(lastTargetOfPrevPair.Flags.SyncPairCount >= 1);
				for (size_t pair = 0; pair < lastTargetOfPrevPair.Flags.SyncPairCount; pair++)
				{
					const auto& prevPairTarget = chart.Targets[(i + pair - lastTargetOfPrevPair.Flags.SyncPairCount)];

					const vec2 screenPosition = renderWindow.TargetAreaToScreenSpace(Rules::TryGetProperties(prevPairTarget).Position);
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
		const vec2 direction = CardinalToTargetRowDirection(cardinal, GetSelectedRowPerBeatDiagonalSpacing());

		const u32 whiteColor = Gui::GetColorU32(ImGuiCol_Text);
		const u32 dimWhiteColor = Gui::GetColorU32(ImGuiCol_Text, 0.35f);
		const u32 dimColor = ImColor(0.1f, 0.1f, 0.1f, 0.75f);

		constexpr auto guideRadius = Rules::TickToDistance(BeatTick::FromBars(1) / 16);
		drawList.AddCircleFilled(row.Start, guideRadius, dimColor, 32);
		drawList.AddCircle(row.Start, guideRadius, whiteColor, 32);

		drawList.AddLine(row.Start, row.Start + (vec2(+0.0f, -1.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(row.Start, row.Start + (vec2(+1.0f, +0.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(row.Start, row.Start + (vec2(+0.0f, +1.0f) * guideRadius), dimWhiteColor, 1.0f);
		drawList.AddLine(row.Start, row.Start + (vec2(-1.0f, +0.0f) * guideRadius), dimWhiteColor, 1.0f);

		static constexpr struct ArrowSettingsData
		{
			f32 Size = 28.0f;
			f32 HeadSpacing = 32.0f;
			f32 HeadEndSpacingFactor = 0.6f;
			f32 HeadSize = 56.0f;
			f32 HeadAngle = 16.0f;
			vec4 BackgroundColor = vec4(0.16f, 0.16f, 0.16f, 0.95f);
		} arrowSettings;

		auto drawArrowHeader = [](ImDrawList& drawList, vec2 position, vec2 direction, ImU32 color, f32 thickness = 1.0f)
		{
			const f32 angleRadians = glm::atan(direction.y, direction.x);

			const f32 headRadiansL = (angleRadians - glm::radians(arrowSettings.HeadAngle));
			const f32 headRadiansR = (angleRadians + glm::radians(arrowSettings.HeadAngle));
			const vec2 headDirectionL = vec2(glm::cos(headRadiansL), glm::sin(headRadiansL));
			const vec2 headDirectionR = vec2(glm::cos(headRadiansR), glm::sin(headRadiansR));
			const u32 backgroundColor = Gui::ColorConvertFloat4ToU32(arrowSettings.BackgroundColor * Gui::ColorConvertU32ToFloat4(color));

			const vec2 headStart = position;
			const vec2 headEnd = headStart + ((arrowSettings.HeadSpacing * arrowSettings.HeadEndSpacingFactor) * direction);

			const vec2 headEndL = headStart + (headDirectionL * (arrowSettings.HeadSize * 0.5f));
			const vec2 headEndR = headStart + (headDirectionR * (arrowSettings.HeadSize * 0.5f));

			drawList.AddLine(headStart, headEnd, backgroundColor, 1.0f);
			drawList.AddTriangleFilled(headStart, headEndL, headEnd, backgroundColor);
			drawList.AddTriangleFilled(headEnd, headEndR, headStart, backgroundColor);

			drawList.AddLine(headStart, headEndL, color, thickness);
			drawList.AddLine(headStart, headEndR, color, thickness);
			drawList.AddLine(headEndL, headEnd, color, thickness);
			drawList.AddLine(headEndR, headEnd, color, thickness);
		};

		const vec2 arrowPosition = row.Start + (direction * (row.Backwards ? (guideRadius - arrowSettings.Size) : guideRadius));

		drawList.AddCircleFilled(row.Start, 2.0f, whiteColor, 9);
		drawList.AddLine(row.Start, arrowPosition, whiteColor, 1.0f);
		drawArrowHeader(drawList, arrowPosition, row.Backwards ? +direction : -direction, whiteColor);

		char textBuffer[32];
		const auto textView = std::string_view(textBuffer, sprintf_s(textBuffer, "[%s]", CardinalDirectionAbbreviations[static_cast<u8>(cardinal)]));
		const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(textView), Gui::StringViewEnd(textView));
		const vec2 textPos = row.Start + vec2(-textSize.x * 0.5f, -guideRadius - textSize.y - 2.0f);

		drawList.AddRectFilled(textPos, textPos + textSize, dimColor);
		drawList.AddText(textPos, whiteColor, Gui::StringViewStart(textView), Gui::StringViewEnd(textView));
	}

	void TargetPositionTool::UpdateKeyboardKeyBindingsInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			const bool local = Gui::GetIO().KeyAlt;;
			if (Gui::IsKeyPressed(KeyBindings::PositionToolFlipHorizontal, false))
				FlipSelectedTargets(undoManager, chart, local ? FlipMode::HorizontalLocal : FlipMode::Horizontal);
			if (Gui::IsKeyPressed(KeyBindings::PositionToolFlipVertical, false))
				FlipSelectedTargets(undoManager, chart, local ? FlipMode::VerticalLocal : FlipMode::Vertical);

			const bool backwards = Gui::GetIO().KeyAlt;
			if (Gui::IsKeyPressed(KeyBindings::PositionToolPositionInRow, false))
				PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, backwards);

			if (Gui::IsKeyPressed(KeyBindings::PositionToolInterpolateLinear, false))
				InterpolateSelectedTargetPositionsLinear(undoManager, chart);

			const bool flip = Gui::GetIO().KeyAlt;
			if (Gui::IsKeyPressed(KeyBindings::PositionToolInterpolateCircular, false))
				InterpolateSelectedTargetPositionsCircular(undoManager, chart, flip ? -1.0f : +1.0f);

			if (Gui::IsKeyPressed(KeyBindings::PositionToolStackPositions, false))
				StackSelectedTargetPositions(undoManager, chart);
		}
	}

	void TargetPositionTool::UpdateKeyboardStepInput(Chart& chart)
	{
		if (selectedTargetsBuffer.empty() || !Gui::IsWindowFocused())
			return;

		for (const auto&[key, direction] : KeyBindings::PositionToolMoveStep)
		{
			// TODO: Improve command merge behavior
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

		const vec2 mousePos = Gui::GetMousePos();

		grab.HoveredTargetID = TimelineTargetID::Null;
		if (Gui::IsWindowHovered() && !selectedTargetsBuffer.empty())
		{
			const auto hoveredTarget = std::find_if(selectedTargetsBuffer.rbegin(), selectedTargetsBuffer.rend(), [&](auto* t)
			{
				const vec2 position = Rules::TryGetProperties(*t).Position;
				const vec2 tl = renderWindow.TargetAreaToScreenSpace(position - TargetRenderWindow::TargetHitboxSize);
				const vec2 br = renderWindow.TargetAreaToScreenSpace(position + TargetRenderWindow::TargetHitboxSize);

				return ImRect(tl, br).Contains(mousePos);
			});

			grab.HoveredTargetID = (hoveredTarget == selectedTargetsBuffer.rend()) ? TimelineTargetID::Null : (*hoveredTarget)->ID;

			if (Gui::IsMouseClicked(0) && (grab.HoveredTargetID != TimelineTargetID::Null))
			{
				grab.GrabbedTargetID = grab.HoveredTargetID;
				grab.MouseOnGrab = mousePos;
				grab.TargetPositionOnGrab = Rules::TryGetProperties(chart.Targets[chart.Targets.FindIndex(grab.GrabbedTargetID)]).Position;
				undoManager.DisallowMergeForLastCommand();
			}
		}

		if (Gui::IsMouseReleased(0))
			grab = {};

		if (Gui::IsMouseDown(0) && (grab.GrabbedTargetID != TimelineTargetID::Null))
		{
			grab.LastPos = grab.ThisPos;
			grab.ThisPos = mousePos;

			grab.LastGridSnap = grab.ThisGridSnap;
			grab.ThisGridSnap = Gui::GetIO().KeyShift;

			Gui::SetActiveID(Gui::GetID(&grab), Gui::GetCurrentWindow());
			Gui::SetWindowFocus();
		}

		if ((grab.HoveredTargetID != TimelineTargetID::Null) || (grab.GrabbedTargetID != TimelineTargetID::Null))
			Gui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

		if (grab.GrabbedTargetID != TimelineTargetID::Null)
		{
			undoManager.ResetMergeTimeThresholdStopwatch();

			if ((grab.ThisPos != grab.LastPos) || (grab.ThisGridSnap != grab.LastGridSnap))
			{
				vec2 grabbedMovedPosition = glm::round(grab.TargetPositionOnGrab + ((grab.ThisPos - grab.MouseOnGrab) / renderWindow.GetCamera().Zoom));
				if (Gui::GetIO().KeyShift)
					grabbedMovedPosition = SnapPositionToGrid(grabbedMovedPosition);

				const vec2 increment = (grabbedMovedPosition - Rules::TryGetProperties(chart.Targets[chart.Targets.FindIndex(grab.GrabbedTargetID)]).Position);
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

			row.SteepLastFrame = row.SteepThisFrame;
			row.SteepThisFrame = Gui::GetIO().KeyShift;

			const bool mouseWasMoved = (Gui::GetIO().MouseDelta.x != 0.0f || Gui::GetIO().MouseDelta.y != 0.0f);
			const bool steepStateChanged = (row.SteepThisFrame != row.SteepLastFrame);

			undoManager.ResetMergeTimeThresholdStopwatch();

			constexpr auto distanceThreshold = 9.0f;
			if (selectedTargetsBuffer.size() > 1 && glm::distance(row.Start, row.End) > distanceThreshold)
			{
				if (row.Start != row.End && (mouseWasMoved || steepStateChanged))
					PositionSelectedTargetsInCardinalRow(undoManager, chart, AngleToNearestCardinal(row.Angle), GetSelectedRowPerBeatDiagonalSpacing(), row.Backwards);
			}
		}
	}

	vec2 TargetPositionTool::GetSelectedRowPerBeatDiagonalSpacing() const
	{
		if (GlobalUserData.PositionTool.DiagonalRowLayouts.empty())
		{
			return vec2(Rules::PlacementDistancePerBeat);
		}
		else
		{
			const auto& selectedLayout = InBounds(selectedDiagonalRowLayoutIndex, GlobalUserData.PositionTool.DiagonalRowLayouts) ?
				GlobalUserData.PositionTool.DiagonalRowLayouts[selectedDiagonalRowLayoutIndex] : GlobalUserData.PositionTool.DiagonalRowLayouts.front();

			return row.SteepThisFrame ? vec2(selectedLayout.PerBeatDiagonalSpacing.y, selectedLayout.PerBeatDiagonalSpacing.x) : selectedLayout.PerBeatDiagonalSpacing;
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
			data.ID = selectedTarget->ID;
			data.NewValue.Position = (Rules::TryGetProperties(*selectedTarget).Position + positionIncrement);
		}

		undoManager.Execute<ChangeTargetListPositions>(chart, std::move(targetData));
	}

	void TargetPositionTool::PositionSelectedTargetsInCardinalRow(Undo::UndoManager& undoManager, Chart& chart, CardinalDirection cardinal, vec2 perBeatDiagonalDirection, bool backwards)
	{
		if (selectedTargetsBuffer.size() < 2)
			return;

		const bool useDiagonalSpacing = IsIntercardinal(cardinal);

		const auto horizontalCardinal =
			(cardinal == CardinalDirection::NorthWest || cardinal == CardinalDirection::SouthWest) ? CardinalDirection::West :
			(cardinal == CardinalDirection::NorthEast || cardinal == CardinalDirection::SouthEast) ? CardinalDirection::East :
			cardinal;

		const vec2 direction = CardinalToTargetRowDirection(cardinal, perBeatDiagonalDirection);
		const vec2 horizontalDirection = CardinalToTargetRowDirection(horizontalCardinal, perBeatDiagonalDirection);

		const f32 distancePerBeat = Rules::PlacementDistancePerBeat;
		const f32 distancePerBeatDiagonal = glm::length(perBeatDiagonalDirection);

		auto getNextPos = [&](vec2 prevPosition, vec2 thisPosition, BeatTick tickDistance, bool chain, bool chainEnd, bool slideHeadTouch) -> vec2
		{
			auto distance = (chain && !chainEnd) ? Rules::ChainFragmentPlacementDistance :
				Rules::TickToDistance(tickDistance, (useDiagonalSpacing && !chainEnd) ? distancePerBeatDiagonal : distancePerBeat);

			if (chainEnd)
				distance += Rules::ChainFragmentStartEndOffsetDistance;
			else if (slideHeadTouch)
				distance += Rules::SlideHeadsTouchOffsetDistance;

			// NOTE: Targets following one after the end of a chain need to be horizontal to avoid decimal fractions.
			//		 If a diagonal pattern is desired then the corret placement would be to vertically offset the height only
			return prevPosition + ((chain ? horizontalDirection : direction) * distance);
		};

		auto doSlideHeadsTouch = [](const TimelineTarget& targetA, const TimelineTarget& targetB, CardinalDirection cardinal) -> bool
		{
			const bool closelySpaced = glm::abs((targetA.Tick - targetB.Tick).Ticks()) <= Rules::SlideHeadsTouchTickThreshold.Ticks();
			const bool slideTypesTouch =
				(cardinal == CardinalDirection::West) ? (targetA.Type == ButtonType::SlideR && targetB.Type == ButtonType::SlideL) :
				(cardinal == CardinalDirection::East) ? (targetA.Type == ButtonType::SlideL && targetB.Type == ButtonType::SlideR) : false;

			return (closelySpaced && slideTypesTouch);
		};

		std::vector<ChangeTargetListPositionsRow::Data> targetData;
		targetData.resize(selectedTargetsBuffer.size());

		if (backwards)
		{
			targetData.back().ID = selectedTargetsBuffer.back()->ID;
			targetData.back().NewValue.Position = Rules::TryGetProperties(*selectedTargetsBuffer.back()).Position;

			for (i32 i = static_cast<i32>(selectedTargetsBuffer.size()) - 2; i >= 0; i--)
			{
				const auto& thisTarget = *selectedTargetsBuffer[i];
				const auto& nextTarget = *selectedTargetsBuffer[i + 1];
				const auto tickDistance = (nextTarget.Tick - thisTarget.Tick);
				const bool slideHeadsTouch = doSlideHeadsTouch(thisTarget, nextTarget, cardinal);

				targetData[i].ID = thisTarget.ID;
				targetData[i].NewValue.Position = getNextPos(targetData[i + 1].NewValue.Position, Rules::TryGetProperties(thisTarget).Position, tickDistance, thisTarget.Flags.IsChain, thisTarget.Flags.IsChainEnd, slideHeadsTouch);
			}
		}
		else
		{
			targetData.front().ID = selectedTargetsBuffer.front()->ID;
			targetData.front().NewValue.Position = Rules::TryGetProperties(*selectedTargetsBuffer.front()).Position;

			for (i32 i = 1; i < static_cast<i32>(selectedTargetsBuffer.size()); i++)
			{
				const auto& thisTarget = *selectedTargetsBuffer[i];
				const auto& prevTarget = *selectedTargetsBuffer[i - 1];
				const auto tickDistance = (thisTarget.Tick - prevTarget.Tick);
				const bool slideHeadsTouch = doSlideHeadsTouch(thisTarget, prevTarget, cardinal);

				targetData[i].ID = thisTarget.ID;
				targetData[i].NewValue.Position = getNextPos(targetData[i - 1].NewValue.Position, Rules::TryGetProperties(thisTarget).Position, tickDistance, prevTarget.Flags.IsChain, prevTarget.Flags.IsChainEnd, slideHeadsTouch);
			}
		}

		undoManager.Execute<ChangeTargetListPositionsRow>(chart, std::move(targetData));
	}

	void TargetPositionTool::FlipSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, FlipMode flipMode)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const bool isHorizontal = (flipMode == FlipMode::Horizontal || flipMode == FlipMode::HorizontalLocal);
		const bool isLocal = (flipMode == FlipMode::HorizontalLocal || flipMode == FlipMode::VerticalLocal);

		const vec2 selectionCenter = std::accumulate(chart.Targets.begin(), chart.Targets.end(), vec2(0.0f),
			[](vec2 p, auto& t) { return t.IsSelected ? p + Rules::TryGetProperties(t).Position : p; }) / static_cast<f32>(selectionCount);

		const vec2 flipCenter = isLocal ? selectionCenter : Rules::PlacementAreaCenter;
		const vec2 componentFlipMask = isHorizontal ? vec2(-1.0f, +1.0f) : vec2(+1.0f, -1.0f);

		std::vector<ChangeTargetListProperties::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const auto properties = Rules::TryGetProperties(target);
			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue = properties;
			data.NewValue.Position = ((properties.Position - flipCenter) * componentFlipMask) + flipCenter;
			data.NewValue.Angle = isHorizontal ? Rules::NormalizeAngle(-data.NewValue.Angle) : Rules::NormalizeAngle(data.NewValue.Angle - 180.0f);
			data.NewValue.Frequency *= -1.0f;
		}

		undoManager.DisallowMergeForLastCommand();
		if (isHorizontal)
			undoManager.Execute<FlipTargetListPropertiesHorizontal>(chart, std::move(targetData));
		else
			undoManager.Execute<FlipTargetListPropertiesVertical>(chart, std::move(targetData));
	}

	void TargetPositionTool::SnapSelectedTargetPositions(Undo::UndoManager& undoManager, Chart& chart, f32 snapDistance)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		std::vector<SnapTargetListPositions::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Position = SnapPositionTo(Rules::TryGetProperties(target).Position, snapDistance);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<SnapTargetListPositions>(chart, std::move(targetData));
	}

	void TargetPositionTool::StackSelectedTargetPositions(Undo::UndoManager& undoManager, Chart& chart)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstFoundTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		const vec2 stackPosition = Rules::TryGetProperties(firstFoundTarget).Position;

		std::vector<StackTargetListPositions::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Position = stackPosition;
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<StackTargetListPositions>(chart, std::move(targetData));
	}

	void TargetPositionTool::PositionSelectedTargetsInRowBetweenFirstAndLastTarget(Undo::UndoManager& undoManager, Chart& chart, bool backwards)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstFoundTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		const auto& lastFoundTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [&](auto& t) { return t.IsSelected; });
		if (firstFoundTarget.Tick == lastFoundTarget.Tick)
			return;

		const vec2 startPosition = Rules::TryGetProperties(firstFoundTarget).Position;
		const vec2 endPosition = Rules::TryGetProperties(lastFoundTarget).Position;
		const vec2 rowDirection = glm::normalize(endPosition - startPosition);

		std::vector<ChangeTargetListPositionsRow::Data> targetData;
		targetData.reserve(selectionCount);

		if (startPosition == endPosition)
		{
			for (const auto& target : chart.Targets)
			{
				if (!target.IsSelected)
					continue;

				auto& data = targetData.emplace_back();
				data.ID = target.ID;
				data.NewValue.Position = startPosition;
			}
		}
		else
		{
			auto applyUsingForwardOrReverseIterators = [this, &chart, &targetData, rowDirection](auto beginIt, auto endIt)
			{
				vec2 prevTargetPosition = {};
				auto prevTargetIt = beginIt;

				for (auto thisTargetIt = beginIt; thisTargetIt != endIt; thisTargetIt++)
				{
					if (!thisTargetIt->IsSelected)
						continue;

					auto& data = targetData.emplace_back();
					data.ID = thisTargetIt->ID;

					if (targetData.size() == 1)
					{
						data.NewValue.Position = Rules::TryGetProperties(*thisTargetIt).Position;
					}
					else
					{
						const auto tickDistance = (thisTargetIt->Tick - prevTargetIt->Tick);

						auto distance = (prevTargetIt->Flags.IsChain && !prevTargetIt->Flags.IsChainEnd) ? Rules::ChainFragmentPlacementDistance : Rules::TickToDistance(tickDistance);
						if (prevTargetIt->Flags.IsChainEnd)
							distance += Rules::ChainFragmentStartEndOffsetDistance;
						else if (/*slideHeadTouch*/false) // NOTE: Don't apply here because it requires knowing the direction which feels beyond this function (?)
							distance += Rules::SlideHeadsTouchOffsetDistance;

						data.NewValue.Position = prevTargetPosition + (rowDirection * distance);
					}

					prevTargetPosition = data.NewValue.Position;
					prevTargetIt = thisTargetIt;
				}
			};

			if (backwards)
				applyUsingForwardOrReverseIterators(chart.Targets.rbegin(), chart.Targets.rend());
			else
				applyUsingForwardOrReverseIterators(chart.Targets.begin(), chart.Targets.end());
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<ChangeTargetListPositionsRow>(chart, std::move(targetData));
	}

	void TargetPositionTool::InterpolateSelectedTargetPositionsLinear(Undo::UndoManager& undoManager, Chart& chart)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstFoundTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		const auto& lastFoundTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [&](auto& t) { return t.IsSelected; });
		if (firstFoundTarget.Tick == lastFoundTarget.Tick)
			return;

		const vec2 startPosition = Rules::TryGetProperties(firstFoundTarget).Position;
		const vec2 endPosition = Rules::TryGetProperties(lastFoundTarget).Position;

		const i32 startTicks = firstFoundTarget.Tick.Ticks();
		const i32 endTicks = lastFoundTarget.Tick.Ticks();
		const f32 tickSpanReciprocal = 1.0f / static_cast<f32>(endTicks - startTicks);

		std::vector<InterpolateTargetListPositions::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const f32 t = static_cast<f32>(target.Tick.Ticks() - startTicks) * tickSpanReciprocal;
			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Position = ((1.0f - t) * startPosition) + (t * endPosition);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListPositions>(chart, std::move(targetData));
	}

	void TargetPositionTool::InterpolateSelectedTargetPositionsCircular(Undo::UndoManager& undoManager, Chart& chart, f32 direction)
	{
		const size_t selectionCount = std::count_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		if (selectionCount < 1)
			return;

		const auto& firstFoundTarget = *std::find_if(chart.Targets.begin(), chart.Targets.end(), [&](auto& t) { return t.IsSelected; });
		const auto& lastFoundTarget = *std::find_if(chart.Targets.rbegin(), chart.Targets.rend(), [&](auto& t) { return t.IsSelected; });
		if (firstFoundTarget.Tick == lastFoundTarget.Tick)
			return;

		const vec2 startPosition = Rules::TryGetProperties(firstFoundTarget).Position;
		const vec2 endPosition = Rules::TryGetProperties(lastFoundTarget).Position;
		const vec2 centerPosition = (startPosition + endPosition) * 0.5f;

		const f32 startAngle = glm::atan(startPosition.y - centerPosition.y, startPosition.x - centerPosition.x);
		const f32 radius = glm::distance(startPosition, centerPosition);

		const i32 startTicks = firstFoundTarget.Tick.Ticks();
		const i32 endTicks = lastFoundTarget.Tick.Ticks();
		const f32 tickSpanReciprocal = 1.0f / static_cast<f32>(endTicks - startTicks);

		std::vector<InterpolateTargetListPositions::Data> targetData;
		targetData.reserve(selectionCount);

		for (const auto& target : chart.Targets)
		{
			if (!target.IsSelected)
				continue;

			const f32 t = static_cast<f32>(target.Tick.Ticks() - startTicks) * tickSpanReciprocal;
			const f32 angle = startAngle - (t * glm::pi<f32>() * direction);
			const vec2 pointOnCircle = vec2(glm::cos(angle), glm::sin(angle)) * radius;

			auto& data = targetData.emplace_back();
			data.ID = target.ID;
			data.NewValue.Position = centerPosition + pointOnCircle;
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListPositions>(chart, std::move(targetData));
	}
}
