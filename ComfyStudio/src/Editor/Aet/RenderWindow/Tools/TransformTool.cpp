#include "TransformTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	static void DrawBoxNode(ImDrawList* drawList, const vec2& position, ImU32 color, float rotation)
	{
		Gui::AddQuadFilled(drawList, position, vec2(TransformBox::NodeRadius), vec2(TransformBox::NodeRadius * 0.5f), rotation, vec2(2.0f), color);
	}

	static void DrawBox(ImDrawList* drawList, const TransformBox& box, bool cross, const vec4& color)
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

	static ImGuiMouseCursor GetCursorForBoxNode(BoxNode boxNode)
	{
		// TODO: Factor in rotation

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
			return 0;
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

	void TransformTool::UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions)
	{
		// TEMP:
		{
			Gui::Text("pos: %.1f %.1f", properties->Position.x, properties->Position.y);
			Gui::Text("ori: %.1f %.1f", properties->Origin.x, properties->Origin.y);
			Gui::Text("rot: %.1f", properties->Rotation);
		}

		ImGuiIO& io = Gui::GetIO();
		bool windowFocused = Gui::IsWindowFocused();
		bool windowHovered = Gui::IsWindowHovered();

		if (windowHovered && Gui::IsMouseClicked(0))
			windowFocused = true;

		vec2 mousePos = io.MousePos;
		vec2 mouseWorldPos = ToWorldSpace(mousePos);

		if (mode == GrabMode::None)
			worldSpaceBox = TransformBox(*properties, dimensions);

		if (windowFocused || windowHovered)
			screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

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
		}

		if (windowFocused  && Gui::IsMouseClicked(actionMouseButton))
		{
			mouseWorldPositionOnMouseDown = mouseWorldPos;
			propertiesOnMouseDown = *properties;

			if (mode == GrabMode::None)
			{
				if (scalingNode >= 0)
				{
					mode = GrabMode::Scale;
					scaleNodeWorldPositionOnMouseDown = mouseWorldPos;
				}
				else if (screenSpaceBox.Contains(mousePos))
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
				// TODO: Round to "grid" (?)
				vec2 grabOffset = mouseWorldPositionOnMouseDown - propertiesOnMouseDown.Position;
				properties->Position = glm::round(mouseWorldPos - grabOffset);

				worldSpaceBox = TransformBox(*properties, dimensions);
			}

			DragPositionTooltip(properties->Position);
		}
		else if (mode == GrabMode::Scale)
		{
			Gui::SetMouseCursor(GetCursorForBoxNode(scalingNode));

			if (allowAction)
			{
				MoveBoxCorner(scalingNode, worldSpaceBox, glm::round(mouseWorldPos), propertiesOnMouseDown.Rotation);
				*properties = worldSpaceBox.GetProperties(dimensions, properties->Origin, properties->Rotation, properties->Opacity);
			}

			DragScaleTooltip(properties->Scale, dimensions);
		}
		else if (mode == GrabMode::None)
		{
			if (hoveringNode >= 0)
				Gui::SetMouseCursor(GetCursorForBoxNode(hoveringNode));
		}

		ImDrawList* drawList = Gui::GetWindowDrawList();

		// box pre grab
		if (mode != GrabMode::None)
		{
			TransformBox worldBoxOnMouseDown = TransformBox(propertiesOnMouseDown, dimensions);
			TransformBox screenBoxOnMouseDown = BoxWorldToScreenSpace(worldBoxOnMouseDown);
			DrawBox(drawList, screenBoxOnMouseDown, false, vec4(1.0f, 1.0f, 1.0f, 0.25f));
		}

		screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

		// base box
		DrawBox(drawList, screenSpaceBox, (mode == GrabMode::Scale), (mode == GrabMode::Move) ? allowAction ? redColor : redPreColor : whiteColor);

		// origin center
		drawList->AddCircleFilled(ToScreenSpace(properties->Position), TransformBox::NodeRadius, ImColor(yellowColor));

		// grab node
		if (mode == GrabMode::Scale)
		{ 
			DrawBoxNode(drawList, screenSpaceBox.GetNodePosition(scalingNode), ImColor(allowAction ? redColor : redPreColor), screenSpaceBox.Rotation());
		}
	}

	void TransformTool::DrawContextMenu()
	{
		// TODO: Origin picker (center, corners etc.)
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