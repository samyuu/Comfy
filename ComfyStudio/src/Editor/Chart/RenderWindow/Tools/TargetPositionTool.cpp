#include "TargetPositionTool.h"
#include "Editor/Chart/ChartCommands.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Core/ComfyStudioSettings.h"
#include <FontIcons.h>
#include <numeric>

namespace Comfy::Studio::Editor
{
	namespace
	{
		vec2 PositionInterpolationSnap(const vec2 position)
		{
			const f32 snapDistance = GlobalUserData.PositionTool.PositionInterpolationCommandSnap;
			return (snapDistance <= 0.0f) ? position : Rules::SnapPositionTo(position, snapDistance);
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

		if (Gui::MenuItem("Flip Targets Horizontally", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_FlipHorizontal).data(), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::Horizontal);
		if (Gui::MenuItem("Flip Targets Horizontally (Local)", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_FlipHorizontalLocal).data(), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::HorizontalLocal);

		if (Gui::MenuItem("Flip Targets Vertically", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_FlipVertical).data(), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::Vertical);
		if (Gui::MenuItem("Flip Targets Vertically (Local)", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_FlipVerticalLocal).data(), false, (lastFrameSelectionCount > 0)))
			FlipSelectedTargets(undoManager, chart, FlipMode::VerticalLocal);

		Gui::Separator();

		if (Gui::BeginMenu("Diagonal Mouse Row Spacing", !GlobalUserData.PositionTool.DiagonalMouseRowLayouts.empty()))
		{
			for (i32 i = 0; i < static_cast<i32>(GlobalUserData.PositionTool.DiagonalMouseRowLayouts.size()); i++)
			{
				const auto& layout = GlobalUserData.PositionTool.DiagonalMouseRowLayouts[i];
				Gui::PushID(&layout);
				char shortcutBuffer[64];
				sprintf_s(shortcutBuffer, "(%g, %g)", layout.PerBeatDiagonalSpacing.x, layout.PerBeatDiagonalSpacing.y);
				if (Gui::MenuItem(layout.DisplayName.c_str(), shortcutBuffer, (i == selectedDiagonalRowLayoutIndex), (i != selectedDiagonalRowLayoutIndex)))
					selectedDiagonalRowLayoutIndex = i;
				Gui::PopID();
			}
			Gui::EndMenu();
		}

		if (Gui::MenuItem("Position in Row", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_PositionInRow).data(), false, (lastFrameSelectionCount > 0)))
			PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, false);
		if (Gui::MenuItem("Position in Row (Back)", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_PositionInRowBack).data(), false, (lastFrameSelectionCount > 0)))
			PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, true);
		Gui::Separator();

		if (Gui::MenuItem("Interpolate Positions Linear", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_InterpolateLinear).data(), false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositionsLinear(undoManager, chart);

		// TODO: Some option to scale the circle X/Y positions (=squash the circle) (?)
		if (Gui::MenuItem("Interpolate Positions Circular", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_InterpolateCircular).data(), false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositionsCircular(undoManager, chart, +1.0f);
		if (Gui::MenuItem("Interpolate Positions Circular (Flip)", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_InterpolateCircularFlip).data(), false, (lastFrameSelectionCount > 0)))
			InterpolateSelectedTargetPositionsCircular(undoManager, chart, -1.0f);
		Gui::Separator();

		if (Gui::MenuItem("Stack Targets", Input::ToString(GlobalUserData.Input.TargetPreview_PositionTool_StackPositions).data(), false, (lastFrameSelectionCount > 0)))
			StackSelectedTargetPositions(undoManager, chart);

		if (Gui::BeginMenu("Snap Positions To", (lastFrameSelectionCount > 0)))
		{
			if (Gui::MenuItem("Nearest Whole", "(1 px)"))
				SnapSelectedTargetPositions(undoManager, chart, 1.0f);

			static_assert(Rules::GridStepDistance == 48.0f);
			if (Gui::MenuItem("Nearest Grid", "(48 px)"))
				SnapSelectedTargetPositions(undoManager, chart, Rules::GridStepDistance);

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
		if (GlobalUserData.PositionTool.ShowDistanceGuides)
			DrawTickDistanceGuides(chart, drawList);

		DrawFlushThisFrameGuides(drawList);
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

	void TargetPositionTool::DrawTickDistanceGuides(Chart& chart, ImDrawList& drawList)
	{
		// NOTE: To both prevent needless visual overload and to not overflow 16bit vertex indices too easily
		const i32 maxGuidesToDraw = GlobalUserData.System.Gui.TargetDistanceGuideMaxCount;
		i32 guideDrawCount = 0;

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
					drawList.AddCircle(screenPosition, screenRadius, GetButtonTypeColorU32(prevPairTarget.Type, 0x84), GlobalUserData.System.Gui.TargetDistanceGuideCircleSegments);

					if (guideDrawCount++ >= maxGuidesToDraw)
						return;
				}
			}

			assert(firstTargetOfPair.Flags.SyncPairCount >= 1);
			i += firstTargetOfPair.Flags.SyncPairCount;
		}
	}

	void TargetPositionTool::DrawFlushThisFrameGuides(ImDrawList& drawList)
	{
		if (thisFrameGuides.empty())
			return;

		if (GlobalUserData.PositionTool.UseAxisSnapGuides)
		{
			for (const auto& guide : thisFrameGuides)
				drawList.AddLine(glm::round(renderWindow.TargetAreaToScreenSpace(guide.Start)), glm::round(renderWindow.TargetAreaToScreenSpace(guide.End)), guide.Color, 1.0f);
		}
		thisFrameGuides.clear();
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

		constexpr f32 guideRadius = Rules::TickToDistance(BeatTick::FromBars(1) / 16);
		drawList.AddCircleFilled(row.Start, guideRadius, dimColor);
		drawList.AddCircle(row.Start, guideRadius, whiteColor);

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

		constexpr vec2 shadowOffset = vec2(1.0f);
		constexpr u32 shadowColor = 0xFF000000;

		const vec2 arrowPosition = row.Start + (direction * (row.Backwards ? (guideRadius - arrowSettings.Size) : guideRadius));

		{
			drawList.AddCircleFilled(row.Start + shadowOffset, 2.0f, shadowColor);
			drawList.AddLine(row.Start + shadowOffset, arrowPosition + shadowOffset, shadowColor, 1.0f);
			drawArrowHeader(drawList, arrowPosition + shadowOffset, row.Backwards ? +direction : -direction, shadowColor);
		}
		{
			drawList.AddCircleFilled(row.Start, 2.0f, whiteColor);
			drawList.AddLine(row.Start, arrowPosition, whiteColor, 1.0f);
			drawArrowHeader(drawList, arrowPosition, row.Backwards ? +direction : -direction, whiteColor);
		}

		{
			char textBuffer[32];
			const auto textView = std::string_view(textBuffer, sprintf_s(textBuffer, "[%s]", CardinalDirectionAbbreviations[static_cast<u8>(cardinal)]));
			const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(textView), Gui::StringViewEnd(textView));
			const vec2 textPos = row.Start + vec2(-textSize.x * 0.5f, -guideRadius - textSize.y - 2.0f);

			drawList.AddRectFilled(textPos, textPos + textSize, dimColor);
			Gui::AddTextWithShadow(&drawList, textPos, textView, whiteColor, shadowColor, shadowOffset);
		}

		if (IsIntercardinal(cardinal) && InBounds(selectedDiagonalRowLayoutIndex, GlobalUserData.PositionTool.DiagonalMouseRowLayouts))
		{
			// NOTE: This is to avoid (accidentally) unintentionally positioning targets with the wrong spacing
			//		 by always making clear which spacing setting is selected
			const auto& selectedDiagonalLayout = GlobalUserData.PositionTool.DiagonalMouseRowLayouts[selectedDiagonalRowLayoutIndex];
			if (selectedDiagonalLayout.PerBeatDiagonalSpacing != Rules::DefaultPerBeatDiagonalSpacing)
			{
				const std::string_view textView = selectedDiagonalLayout.DisplayName;
				const vec2 textSize = Gui::CalcTextSize(Gui::StringViewStart(textView), Gui::StringViewEnd(textView));
				const vec2 textPos = row.Start + vec2(-textSize.x * 0.5f, guideRadius + 2.0f);

				drawList.AddRectFilled(textPos, textPos + textSize, dimColor);
				Gui::AddTextWithShadow(&drawList, textPos, textView, whiteColor, shadowColor, shadowOffset);
			}
		}
	}

	void TargetPositionTool::UpdateKeyboardKeyBindingsInput(Chart& chart)
	{
		if (Gui::IsWindowFocused() && Gui::GetActiveID() == 0)
		{
			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_FlipHorizontalLocal, false))
				FlipSelectedTargets(undoManager, chart, FlipMode::HorizontalLocal);
			else if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_FlipHorizontal, false))
				FlipSelectedTargets(undoManager, chart, FlipMode::Horizontal);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_FlipVerticalLocal, false))
				FlipSelectedTargets(undoManager, chart, FlipMode::VerticalLocal);
			else if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_FlipVertical, false))
				FlipSelectedTargets(undoManager, chart, FlipMode::Vertical);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_PositionInRowBack, false))
				PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, true);
			else if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_PositionInRow, false))
				PositionSelectedTargetsInRowBetweenFirstAndLastTarget(undoManager, chart, false);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_InterpolateLinear, false))
				InterpolateSelectedTargetPositionsLinear(undoManager, chart);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_InterpolateCircularFlip, false))
				InterpolateSelectedTargetPositionsCircular(undoManager, chart, -1.0f);
			else if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_InterpolateCircular, false))
				InterpolateSelectedTargetPositionsCircular(undoManager, chart, +1.0f);

			if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_StackPositions, false))
				StackSelectedTargetPositions(undoManager, chart);
		}
	}

	void TargetPositionTool::UpdateKeyboardStepInput(Chart& chart)
	{
		if (selectedTargetsBuffer.empty() || !Gui::IsWindowFocused())
			return;

		if (Gui::GetActiveID() != 0)
			return;

		auto moveInDirection = [&](vec2 direction)
		{
			const auto& io = Gui::GetIO();
			const f32 stepDistance =
				io.KeyShift ? GlobalUserData.PositionTool.PositionKeyMoveStepRough :
				io.KeyAlt ? GlobalUserData.PositionTool.PositionKeyMoveStepPrecise : GlobalUserData.PositionTool.PositionKeyMoveStep;

			// TODO: Improve command merge behavior (?)
			IncrementSelectedTargetPositionsBy(undoManager, chart, direction * stepDistance);
		};

		if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_MoveUp, true, Input::ModifierBehavior_Relaxed))
			moveInDirection(vec2(+0.0f, -1.0f));
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_MoveLeft, true, Input::ModifierBehavior_Relaxed))
			moveInDirection(vec2(-1.0f, +0.0f));
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_MoveDown, true, Input::ModifierBehavior_Relaxed))
			moveInDirection(vec2(+0.0f, +1.0f));
		if (Input::IsAnyPressed(GlobalUserData.Input.TargetPreview_PositionTool_MoveRight, true, Input::ModifierBehavior_Relaxed))
			moveInDirection(vec2(+1.0f, +0.0f));
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

			if (Gui::IsMouseClicked(ImGuiMouseButton_Left) && (grab.HoveredTargetID != TimelineTargetID::Null))
			{
				grab.GrabbedTargetID = grab.HoveredTargetID;
				grab.MouseOnGrab = mousePos;
				grab.TargetPositionOnGrab = Rules::TryGetProperties(chart.Targets[chart.Targets.FindIndex(grab.GrabbedTargetID)]).Position;
				undoManager.DisallowMergeForLastCommand();
			}
		}

		if (Gui::IsMouseReleased(ImGuiMouseButton_Left))
			grab = {};

		if (Gui::IsMouseDown(ImGuiMouseButton_Left) && (grab.GrabbedTargetID != TimelineTargetID::Null))
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

			const auto& grabbedTarget = chart.Targets[chart.Targets.FindIndex(grab.GrabbedTargetID)];
			if (GlobalUserData.PositionTool.UseAxisSnapGuides)
				SetupAxisGrabGuides(grabbedTarget);

			if ((grab.ThisPos != grab.LastPos) || (grab.ThisGridSnap != grab.LastGridSnap))
			{
				// TODO: Make modifiers configurable too (?)
				const auto& io = Gui::GetIO();
				const f32 positionSnap =
					io.KeyShift ? GlobalUserData.PositionTool.PositionMouseSnapRough :
					io.KeyAlt ? GlobalUserData.PositionTool.PositionMouseSnapPrecise : GlobalUserData.PositionTool.PositionMouseSnap;

				vec2 grabMovedPosition = grab.TargetPositionOnGrab + ((grab.ThisPos - grab.MouseOnGrab) / renderWindow.GetCamera().Zoom);
				if (GlobalUserData.PositionTool.UseAxisSnapGuides)
					grabMovedPosition = TrySnapGrabPositionToGuides(grabMovedPosition);
				grabMovedPosition = Rules::SnapPositionTo(grabMovedPosition, positionSnap);

				const vec2 positionIncrement = (grabMovedPosition - Rules::TryGetProperties(grabbedTarget).Position);

				if (positionIncrement != vec2(0.0f))
					IncrementSelectedTargetPositionsBy(undoManager, chart, positionIncrement);
			}

			if (GlobalUserData.PositionTool.ShowTargetGrabTooltip)
				GuiTargetGrabTooltip(chart);
		}
	}

	void TargetPositionTool::UpdateMouseRowInput(Chart& chart)
	{
		if (!selectedTargetsBuffer.empty() && Gui::IsWindowHovered() && Gui::IsWindowFocused())
		{
			if (Gui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				row.Start = Gui::GetMousePos();
				row.Active = true;
				row.Backwards = Gui::GetIO().KeyAlt;
				undoManager.DisallowMergeForLastCommand();
			}
		}

		if (Gui::IsMouseReleased(ImGuiMouseButton_Left))
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

			if (selectedTargetsBuffer.size() > 1 && glm::distance(row.Start, row.End) > GlobalUserData.PositionTool.MouseRowCenterDistanceThreshold)
			{
				if (row.Start != row.End && (mouseWasMoved || steepStateChanged))
					PositionSelectedTargetsInCardinalRow(undoManager, chart, AngleToNearestCardinal(row.Angle), GetSelectedRowPerBeatDiagonalSpacing(), row.Backwards);
			}
		}
	}

	void TargetPositionTool::SetupAxisGrabGuides(const TimelineTarget& grabbedTarget)
	{
		const vec2 grabbedTargetPosition = Rules::TryGetProperties(grabbedTarget).Position;
		const vec2 movedSoFar = (grabbedTargetPosition - grab.TargetPositionOnGrab);

		static constexpr u8 guideAlpha = 0xDD;
		auto addVerticalGuide = [this](f32 x, ButtonType type) { thisFrameGuides.push_back({ vec2(x, 0.0f), vec2(x, Rules::PlacementAreaSize.y), GetButtonTypeColorU32(type, guideAlpha) }); };
		auto addhorizontalGuide = [this](f32 y, ButtonType type) { thisFrameGuides.push_back({ vec2(0.0f, y), vec2(Rules::PlacementAreaSize.x, y), GetButtonTypeColorU32(type, guideAlpha) }); };

		if (movedSoFar.x == 0.0f)
			addVerticalGuide(grabbedTargetPosition.x, grabbedTarget.Type);
		if (movedSoFar.y == 0.0f)
			addhorizontalGuide(grabbedTargetPosition.y, grabbedTarget.Type);
	}

	vec2 TargetPositionTool::TrySnapGrabPositionToGuides(vec2 grabMovedPosition) const
	{
		const f32 snapThreshold = GlobalUserData.PositionTool.AxisSnapGuideDistanceThreshold;

		for (const auto& guide : thisFrameGuides)
		{
			for (vec2::length_type component = 0; component < vec2::length(); component++)
			{
				if (guide.Start[component] == guide.End[component])
				{
					if (glm::distance(guide.Start[component], grabMovedPosition[component]) <= snapThreshold)
						grabMovedPosition[component] = guide.Start[component];
					break;
				}
			}
		}

		return grabMovedPosition;
	}

	void TargetPositionTool::GuiTargetGrabTooltip(const Chart& chart) const
	{
		const auto& grabbedTarget = chart.Targets[chart.Targets.FindIndex(grab.GrabbedTargetID)];
		const vec2 grabbedTargetPosition = Rules::TryGetProperties(grabbedTarget).Position;
		const vec2 movedSoFar = (grabbedTargetPosition - grab.TargetPositionOnGrab);
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(3.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
		{
			Gui::PushStyleColor(ImGuiCol_PopupBg, Gui::GetColorU32(ImGuiCol_PopupBg, 0.85f));
			Gui::PushStyleColor(ImGuiCol_Border, Gui::GetColorU32(ImGuiCol_BorderShadow, 4.0f));
			Gui::PushStyleColor(ImGuiCol_Separator, Gui::GetColorU32(ImGuiCol_BorderShadow, 3.0f));
			Gui::BeginTooltip();
			{
				Gui::BeginChild("ColumnChild", vec2(112.0f, 52.0f + 2.0f), false, ImGuiWindowFlags_NoBackground);
				Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_None);
				Gui::SetColumnWidth(0, 25.0f + 2.0f);

				Gui::TextUnformatted(" " ICON_FA_ARROWS_ALT); Gui::NextColumn(); Gui::Text(":  %4.f, %4.f px", grabbedTargetPosition.x, grabbedTargetPosition.y); Gui::NextColumn();
				Gui::Separator();
				Gui::TextUnformatted(movedSoFar.x < 0.0f ? " " ICON_FA_ARROW_LEFT : " " ICON_FA_ARROW_RIGHT); Gui::NextColumn(); Gui::Text(":  %4.f px", glm::abs(movedSoFar.x)); Gui::NextColumn();
				Gui::TextUnformatted(movedSoFar.y < 0.0f ? " " ICON_FA_ARROW_UP : " " ICON_FA_ARROW_DOWN); Gui::NextColumn(); Gui::Text(":  %4.f px", glm::abs(movedSoFar.y)); Gui::NextColumn();

				Gui::EndColumns();
				Gui::EndChild();
			}
			Gui::EndTooltip();
			Gui::PopStyleColor(3);
		}
		Gui::PopStyleVar(2);
	}

	vec2 TargetPositionTool::GetSelectedRowPerBeatDiagonalSpacing() const
	{
		const vec2 perBeaySpacing = InBounds(selectedDiagonalRowLayoutIndex, GlobalUserData.PositionTool.DiagonalMouseRowLayouts) ?
			GlobalUserData.PositionTool.DiagonalMouseRowLayouts[selectedDiagonalRowLayoutIndex].PerBeatDiagonalSpacing : Rules::DefaultPerBeatDiagonalSpacing;

		return row.SteepThisFrame ? vec2(perBeaySpacing.y, perBeaySpacing.x) : perBeaySpacing;
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
			data.NewValue.Angle = isHorizontal ? Rules::NormalizeAngle(-data.NewValue.Angle) : Rules::NormalizeAngle(180.0f - data.NewValue.Angle);
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
			data.NewValue.Position = Rules::SnapPositionTo(Rules::TryGetProperties(target).Position, snapDistance);
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
				data.NewValue.Position = PositionInterpolationSnap(startPosition);
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
						data.NewValue.Position = PositionInterpolationSnap(Rules::TryGetProperties(*thisTargetIt).Position);
					}
					else
					{
						const auto tickDistance = (thisTargetIt->Tick - prevTargetIt->Tick);

						auto distance = (prevTargetIt->Flags.IsChain && !prevTargetIt->Flags.IsChainEnd) ? Rules::ChainFragmentPlacementDistance : Rules::TickToDistance(tickDistance);
						if (prevTargetIt->Flags.IsChainEnd)
							distance += Rules::ChainFragmentStartEndOffsetDistance;
						else if (/*slideHeadTouch*/false) // NOTE: Don't apply here because it requires knowing the direction which feels beyond this function (?)
							distance += Rules::SlideHeadsTouchOffsetDistance;

						data.NewValue.Position = PositionInterpolationSnap(prevTargetPosition + (rowDirection * distance));
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
			data.NewValue.Position = PositionInterpolationSnap(((1.0f - t) * startPosition) + (t * endPosition));
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
			data.NewValue.Position = PositionInterpolationSnap(centerPosition + pointOnCircle);
		}

		undoManager.DisallowMergeForLastCommand();
		undoManager.Execute<InterpolateTargetListPositions>(chart, std::move(targetData));
	}
}
