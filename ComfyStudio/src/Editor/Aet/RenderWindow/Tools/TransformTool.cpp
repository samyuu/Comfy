#include "TransformTool.h"
#include "Editor/Aet/AetIcons.h"
#include "Editor/Aet/Command/Commands.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	namespace
	{
		void DrawBoxNode(ImDrawList* drawList, const vec2& position, ImU32 color, float rotation)
		{
			Gui::AddQuadFilled(drawList, position, vec2(TransformBox::NodeRadius), vec2(TransformBox::NodeRadius * 0.5f), rotation, vec2(2.0f), color);
		}

		void DrawBox(ImDrawList* drawList, const TransformBox& box, bool cross, const vec4& color)
		{
			ImU32 lineColor = ImColor(vec4(color.r, color.g, color.b, color.a * 0.75f));

			Gui::AddLine(drawList, box.TL, box.TR, lineColor);
			Gui::AddLine(drawList, box.TR, box.BR, lineColor);
			Gui::AddLine(drawList, box.BR, box.BL, lineColor);
			Gui::AddLine(drawList, box.BL, box.TL, lineColor);

			if (cross)
			{
				Gui::AddLine(drawList, box.TL, box.BR, lineColor);
				Gui::AddLine(drawList, box.TR, box.BL, lineColor);
			}

			ImU32 nodeColor = ImColor(color);
			float rotation = box.Rotation();

			DrawBoxNode(drawList, box.TL, nodeColor, rotation);
			DrawBoxNode(drawList, box.TR, nodeColor, rotation);
			DrawBoxNode(drawList, box.BL, nodeColor, rotation);
			DrawBoxNode(drawList, box.BR, nodeColor, rotation);

			DrawBoxNode(drawList, box.Top(), nodeColor, rotation);
			DrawBoxNode(drawList, box.Right(), nodeColor, rotation);
			DrawBoxNode(drawList, box.Bottom(), nodeColor, rotation);
			DrawBoxNode(drawList, box.Left(), nodeColor, rotation);
		}

		float AngleBetween(const vec2& pointA, const vec2& pointB)
		{
			return glm::degrees(glm::atan(pointA.y - pointB.y, pointA.x - pointB.x));
		}

		ImGuiMouseCursor GetBoxNodeCursor(const TransformBox& box, BoxNode boxNode)
		{
			switch (boxNode)
			{
			case BoxNode_TL:
			case BoxNode_BR:
				return ImGuiMouseCursor_ResizeNWSE;
			case BoxNode_TR:
			case BoxNode_BL:
				return ImGuiMouseCursor_ResizeNESW;
			case BoxNode_Top:
			case BoxNode_Bottom:
				return ImGuiMouseCursor_ResizeNS;
			case BoxNode_Right:
			case BoxNode_Left:
				return ImGuiMouseCursor_ResizeEW;
			default:
				return ImGuiMouseCursor_None;
			}

			// TODO:
			float angle = AngleBetween(box.Center(), box.GetNodePosition(boxNode));
			angle = fmod(angle + 270.0f, 360.0f);

			constexpr float threshold = 45.0f * 0.5f;

			// "|"
			if (angle >= (360 - threshold) || angle <= (0 + threshold))
				return ImGuiMouseCursor_ResizeNS;
			if (angle >= (180 - threshold) && angle <= (180 + threshold))
				return ImGuiMouseCursor_ResizeNS;

			// "-"
			if (angle >= (90 - threshold) && angle <= (90 + threshold))
				return ImGuiMouseCursor_ResizeEW;
			if (angle >= (270 - threshold) && angle <= (270 + threshold))
				return ImGuiMouseCursor_ResizeEW;

			// "\"
			if (angle >= (315 - threshold) && angle <= (315 + threshold))
				return ImGuiMouseCursor_ResizeNWSE;
			if (angle >= (135 - threshold) && angle <= (135 + threshold))
				return ImGuiMouseCursor_ResizeNWSE;

			// "/"
			if (angle >= (45 - threshold) && angle <= (45 + threshold))
				return ImGuiMouseCursor_ResizeNESW;
			if (angle >= (225 - threshold) && angle <= (225 + threshold))
				return ImGuiMouseCursor_ResizeNESW;

			assert(false);
			return ImGuiMouseCursor_None;
		}
	}

	const char* TransformTool::GetIcon() const
	{
		return ICON_FA_EXPAND;
	}

	const char* TransformTool::GetName() const
	{
		return "Transform Tool";
	}

	AetToolType TransformTool::GetType() const
	{
		return AetToolType_Transform;
	}

	KeyCode TransformTool::GetShortcutKey() const
	{
		return KeyCode_T;
	}

	void TransformTool::UpdatePostDrawGui(Transform2D* transform, vec2 dimensions)
	{
		ImGuiIO& io = Gui::GetIO();
		bool windowFocused = Gui::IsWindowFocused();
		bool windowHovered = Gui::IsWindowHovered();

		if (windowHovered && Gui::IsMouseClicked(0))
			windowFocused = true;

		vec2 mousePos = io.MousePos;
		vec2 mouseWorldPos = glm::round(ToWorldSpace(mousePos));

		if (mode == GrabMode::None)
			worldSpaceBox = TransformBox(*transform, dimensions);

		if (windowFocused || windowHovered)
			screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

		if (windowFocused)
			UpdateKeyboardMoveInput(transform);

		if (windowFocused && Gui::IsMouseClicked(actionMouseButton))
		{
			for (int i = 0; i < BoxNode_Count; i++)
			{
				if (glm::distance(screenSpaceBox.GetNodePosition(static_cast<BoxNode>(i)), mousePos) < TransformBox::NodeHitboxRadius)
					scalingNode = static_cast<BoxNode>(i);
			}
		}

		hoveringNode = BoxNode_Invalid;
		if (windowHovered)
		{
			for (int i = 0; i < BoxNode_Count; i++)
			{
				if (glm::distance(screenSpaceBox.GetNodePosition(static_cast<BoxNode>(i)), mousePos) < TransformBox::NodeHitboxRadius)
					hoveringNode = static_cast<BoxNode>(i);
			}

			boxHovered = screenSpaceBox.Contains(mousePos);
		}
		else
		{
			boxHovered = false;
		}

		if (windowFocused  && Gui::IsMouseClicked(actionMouseButton))
		{
			mouseWorldPositionOnMouseDown = mouseWorldPos;
			transformOnMouseDown = *transform;

			if (mode == GrabMode::None)
			{
				if (scalingNode >= 0)
				{
					mode = GrabMode::Scale;
					scaleNodeWorldPositionOnMouseDown = mouseWorldPos;
				}
				else if (boxHovered)
				{
					mode = GrabMode::Move;
				}
			}
		}

		// NOTE: Make sure clicking on a node won't resize immediately until the mouse has been moved a bit
		if (mode != GrabMode::None && !allowAction && Gui::IsMouseDragging(actionMouseButton, mouseDragThreshold))
			allowAction = true;

		if (!windowFocused || Gui::IsMouseReleased(actionMouseButton))
		{
			mode = GrabMode::None;
			scalingNode = BoxNode_Invalid;
			allowAction = false;
		}

		if (mode == GrabMode::Move)
		{
			Gui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

			if (allowAction)
			{
				vec2 grabOffset = mouseWorldPositionOnMouseDown - transformOnMouseDown.Position;
				transform->Position = glm::round(mouseWorldPos - grabOffset);

				if (Gui::IsKeyDown(GridSnapModifierKey))
					transform->Position = Snap(transform->Position, PositionSnapPrecision);

				worldSpaceBox = TransformBox(*transform, dimensions);
			}

			DragPositionTooltip(transform->Position);
		}
		else if (mode == GrabMode::Scale)
		{
			Gui::SetMouseCursor(GetBoxNodeCursor(worldSpaceBox, scalingNode));

			if (allowAction)
			{
				vec2 newNodePosition = mouseWorldPos;

				if (Gui::IsKeyDown(GridSnapModifierKey))
					newNodePosition = Snap(newNodePosition, ScaleSnapPrecision);

				MoveBoxCorner(scalingNode, worldSpaceBox, newNodePosition, transformOnMouseDown.Rotation);
				*transform = worldSpaceBox.GetTransform(dimensions, transform->Origin, transform->Rotation, transform->Opacity);
			}

			DragScaleTooltip(transform->Scale, dimensions);
		}
		else if (mode == GrabMode::None)
		{
			if (hoveringNode != BoxNode_Invalid)
				Gui::SetMouseCursor(GetBoxNodeCursor(worldSpaceBox, hoveringNode));
		}

		ImDrawList* drawList = Gui::GetWindowDrawList();

		// NOTE: Draw box pre grab
		if (mode != GrabMode::None)
		{
			TransformBox worldBoxOnMouseDown = TransformBox(transformOnMouseDown, dimensions);
			TransformBox screenBoxOnMouseDown = BoxWorldToScreenSpace(worldBoxOnMouseDown);
			DrawBox(drawList, screenBoxOnMouseDown, false, vec4(1.0f, 1.0f, 1.0f, 0.25f));
		}

		screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

		// NOTE: Draw base box
		DrawBox(drawList, screenSpaceBox, (mode == GrabMode::Scale), (mode == GrabMode::Move) ? allowAction ? redColor : redPreColor : whiteColor);

		// NOTE: Draw origin point
		drawList->AddCircleFilled(ToScreenSpace(transform->Position), TransformBox::NodeRadius, ImColor(yellowColor));

		// NOTE: Draw grabbing node
		if (mode == GrabMode::Scale)
			DrawBoxNode(drawList, screenSpaceBox.GetNodePosition(scalingNode), ImColor(allowAction ? redColor : redPreColor), screenSpaceBox.Rotation());
	}

	void TransformTool::ProcessCommands(AetCommandManager* commandManager, const RefPtr<Aet::Layer>& layer, float frame, const Transform2D& transform, const Transform2D& previousTransform)
	{
		if (transform == previousTransform)
			return;

		// NOTE: Is this the desired behavior (?)
		if (transform.Scale == previousTransform.Scale)
		{
			const auto tuple = std::make_tuple(frame, transform.Position);
			ProcessUpdatingAetCommand(commandManager, AnimationDataChangePosition, layer, tuple);
		}
		else
		{
			const auto tuple = std::make_tuple(frame, transform.Position, transform.Scale);
			ProcessUpdatingAetCommand(commandManager, AnimationDataChangeTransform, layer, tuple);
		}
	}

	void TransformTool::DrawContextMenu()
	{
		// TODO: Origin picker (center, corners etc.)
	}

	bool TransformTool::MouseFocusCaptured() const
	{
		return (hoveringNode != BoxNode_Invalid) || (scalingNode != BoxNode_Invalid) || (boxHovered) || (allowAction);
	}

	void TransformTool::UpdateKeyboardMoveInput(Transform2D* transform)
	{
		// TODO: This should probably be moved into a common method so the MoveTool can also use it

		struct NudgeBinding
		{
			i16 Key;
			enum Component : u8 { X, Y } Component;
			enum Direction : u8 { Increment, Decrement } Direction;
		};

		constexpr NudgeBinding bindings[] =
		{
			{ KeyCode_Up,	 NudgeBinding::Y, NudgeBinding::Decrement },
			{ KeyCode_Down,	 NudgeBinding::Y, NudgeBinding::Increment },
			{ KeyCode_Left,	 NudgeBinding::X, NudgeBinding::Decrement },
			{ KeyCode_Right, NudgeBinding::X, NudgeBinding::Increment },
		};

		for (auto& binding : bindings)
		{
			if (Gui::IsKeyPressed(binding.Key))
			{
				float step = Gui::IsKeyDown(FastNudgeModifierKey) ? NudgeFastStepDistance : NudgeStepDistance;

				if (binding.Direction == NudgeBinding::Decrement)
					step *= -1.0f;

				transform->Position[binding.Component] += step;
			}
		}
	}

	void TransformTool::MoveBoxCorner(BoxNode scalingNode, TransformBox& box, vec2 position, float rotation) const
	{
		const TransformBox originalBox = box;

		if (rotation != 0.0f)
		{
			float radians = glm::radians(-rotation), sin = glm::sin(radians), cos = glm::cos(radians);

			for (auto& corner : box.Corners)
				RotatePointSinCos(corner, sin, cos);

			RotatePointSinCos(position, sin, cos);
		}

		if (scalingNode >= BoxNode_TL && scalingNode <= BoxNode_BR)
		{
			int cornerNodes[2][4] = { { BoxNode_BL, BoxNode_BR, BoxNode_TL, BoxNode_TR }, { BoxNode_TR, BoxNode_TL, BoxNode_BR, BoxNode_BL } };

			box.Corners[scalingNode] = position;
			for (int i = 0; i < 2; i++)
				box.Corners[cornerNodes[i][scalingNode]][i] = position[i];
		}
		else if (scalingNode >= BoxNode_Top && scalingNode <= BoxNode_Left)
		{
			int edgeNodes[2][4] = { { BoxNode_TL, BoxNode_TR, BoxNode_BL, BoxNode_TL }, { BoxNode_TR, BoxNode_BR, BoxNode_BR, BoxNode_BL } };

			for (int i = 0; i < 2; i++)
				box.Corners[edgeNodes[i][scalingNode - BoxNode_Top]][~scalingNode & 1] = position[~scalingNode & 1];
		}

		if (rotation != 0.0f)
		{
			float radians = glm::radians(rotation), sin = glm::sin(radians), cos = glm::cos(radians);

			for (auto& corner : box.Corners)
				RotatePointSinCos(corner, sin, cos);
		}
	}

	void TransformTool::DragPositionTooltip(const vec2& position)
	{
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(3.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
		{
			vec4 popupBg = Gui::GetStyleColorVec4(ImGuiCol_PopupBg);
			popupBg.a *= .85f;
			Gui::PushStyleColor(ImGuiCol_PopupBg, popupBg);
			{
				Gui::BeginTooltip();
				{
					Gui::Text(ICON_FA_ARROWS_ALT_H "   :  %.f px", position.x);
					Gui::Text(ICON_FA_ARROWS_ALT_V "   :  %.f px", position.y);
				}
				Gui::EndTooltip();
			}
			Gui::PopStyleColor();
		}
		Gui::PopStyleVar(2);
	}

	void TransformTool::DragScaleTooltip(const vec2& scale, const vec2& dimensions)
	{
		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(3.0f, 3.0f));
		Gui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
		{
			vec4 popupBg = Gui::GetStyleColorVec4(ImGuiCol_PopupBg);
			popupBg.a *= .85f;
			Gui::PushStyleColor(ImGuiCol_PopupBg, popupBg);
			{
				Gui::BeginTooltip();
				{
					bool cornerNode = scalingNode >= BoxNode_TL && scalingNode <= BoxNode_BR;
					bool verticalNode = scalingNode == BoxNode_Top || scalingNode == BoxNode_Bottom;
					bool horizontalNode = scalingNode == BoxNode_Left || scalingNode == BoxNode_Right;

					if (cornerNode || horizontalNode)
						Gui::Text("X  :  %.f %%", scale.x * 100.0f);
					if (cornerNode || verticalNode)
						Gui::Text("Y  :  %.f %%", scale.y * 100.0f);
				}
				Gui::EndTooltip();
			}
			Gui::PopStyleColor();
		}
		Gui::PopStyleVar(2);
	}

	TransformBox TransformTool::BoxWorldToScreenSpace(const TransformBox& box) const
	{
		TransformBox screenSpaceBox;
		for (int i = 0; i < sizeof(box.Corners) / sizeof(vec2); i++)
			screenSpaceBox.Corners[i] = ToScreenSpace(box.Corners[i]);
		return screenSpaceBox;
	}
}
