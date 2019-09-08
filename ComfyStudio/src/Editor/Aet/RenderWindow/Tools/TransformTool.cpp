#include "TransformTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	static void DrawBox(ImDrawList* drawList, const TransformBox& box, bool cross, const vec4& color)
	{
		ImU32 colorU32 = ImColor(color);
		drawList->AddLine(box.TL, box.TR, colorU32);
		drawList->AddLine(box.TR, box.BR, colorU32);
		drawList->AddLine(box.BR, box.BL, colorU32);
		drawList->AddLine(box.BL, box.TL, colorU32);

		if (cross)
		{
			drawList->AddLine(box.TL, box.BR, colorU32);
			drawList->AddLine(box.TR, box.BL, colorU32);
		}

		drawList->AddCircleFilled(box.TL, TransformBox::NodeRadius, colorU32);
		drawList->AddCircleFilled(box.TR, TransformBox::NodeRadius, colorU32);
		drawList->AddCircleFilled(box.BL, TransformBox::NodeRadius, colorU32);
		drawList->AddCircleFilled(box.BR, TransformBox::NodeRadius, colorU32);

		drawList->AddCircleFilled(box.Top(), TransformBox::NodeRadius, colorU32);
		drawList->AddCircleFilled(box.Right(), TransformBox::NodeRadius, colorU32);
		drawList->AddCircleFilled(box.Bottom(), TransformBox::NodeRadius, colorU32);
		drawList->AddCircleFilled(box.Left(), TransformBox::NodeRadius, colorU32);
	}

	static ImGuiMouseCursor GetCursorForBoxNode(int boxNode)
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
		return ICON_FA_EXPAND_ARROWS_ALT;
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
		Gui::Text("pos: %.1f %.1f", properties->Position.x, properties->Position.y);
		Gui::Text("ori: %.1f %.1f", properties->Origin.x, properties->Origin.y);

		constexpr vec4 redColor = vec4(1.0f, .25f, .25f, 0.85f);
		constexpr vec4 yellowColor = vec4(.75f, .75f, .25f, 0.85f);
		constexpr vec4 whiteColor = vec4(1.0f, 1.0f, 1.0f, 0.85f);

		ImGuiIO& io = Gui::GetIO();
		bool windowFocused = Gui::IsWindowFocused();
		bool windowHovered = Gui::IsWindowHovered();

		vec2 mousePos = io.MousePos;
		vec2 mouseWorldPos = ToWorldSpace(mousePos);

		if (mode == GrabMode::None)
			worldSpaceBox = TransformBox(*properties, dimensions);

		if (windowFocused || windowHovered)
			screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

		if (windowFocused && Gui::IsMouseClicked(0))
		{
			for (int i = 0; i < BoxNode_Count; i++)
			{
				if (glm::distance(screenSpaceBox.GetNodePosition(i), mousePos) < TransformBox::NodeRadius)
					scalingNode = i;
			}
		}

		hoveringNode = -1;
		if (windowHovered)
		{
			for (int i = 0; i < BoxNode_Count; i++)
			{
				if (glm::distance(screenSpaceBox.GetNodePosition(i), mousePos) < TransformBox::NodeRadius)
					hoveringNode = i;
			}
		}

		if (windowFocused && Gui::IsMouseClicked(0))
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
		if (!windowFocused || Gui::IsMouseReleased(0))
		{
			mode = GrabMode::None;
			scalingNode = -1;
		}

		if (mode == GrabMode::Move)
		{
			Gui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

			// TODO: round to "grid"
			vec2 grabOffset = mouseWorldPositionOnMouseDown - propertiesOnMouseDown.Position;
			properties->Position = glm::round(mouseWorldPos - grabOffset);

			worldSpaceBox = TransformBox(*properties, dimensions);

			DragPositionTooltip(properties->Position);
		}
		else if (mode == GrabMode::Scale)
		{
			Gui::SetMouseCursor(GetCursorForBoxNode(scalingNode));

			MoveBoxCorner(worldSpaceBox, glm::round(mouseWorldPos), propertiesOnMouseDown.Rotation);
			DragScaleTooltip(properties->Scale, dimensions);

			*properties = worldSpaceBox.GetProperties(dimensions, properties->Origin, properties->Rotation, properties->Opacity);
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
		DrawBox(drawList, screenSpaceBox, (mode == GrabMode::Scale), mode == GrabMode::Move ? redColor : whiteColor);

		// origin center
		drawList->AddCircleFilled(ToScreenSpace(properties->Position), TransformBox::NodeRadius, ImColor(yellowColor));

		// grab node
		if (mode == GrabMode::Scale)
			drawList->AddCircle(screenSpaceBox.GetNodePosition(scalingNode), TransformBox::NodeRadius, ImColor(redColor));
	}

	void TransformTool::DrawContextMenu()
	{
		Gui::MenuItem("TODO: Origin picker (center, corners etc.)");
	}

	void TransformTool::MoveBoxCorner(TransformBox& box, vec2 position, float rotation)
	{
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
		else
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