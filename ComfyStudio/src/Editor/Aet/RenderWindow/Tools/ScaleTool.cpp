#include "ScaleTool.h"
#include "Editor/Aet/AetIcons.h"

namespace Editor
{
	static ImGuiMouseCursor GetCursorForScaleNode(ScaleNode scaleNode)
	{
		// TODO: Factor in rotation

		switch (scaleNode)
		{
		case ScaleNode_AxisX:
			return ImGuiMouseCursor_ResizeEW;
		case ScaleNode_AxisY:
			return ImGuiMouseCursor_ResizeNS;
		case ScaleNode_AxisXY:
		default:
			return 0;
		}
	}

	const char* ScaleTool::GetIcon() const
	{
		return ICON_FA_COMPRESS_ARROWS_ALT;
	}

	const char* ScaleTool::GetName() const
	{
		return "Scale Tool";
	}

	AetToolType ScaleTool::GetType() const
	{
		return AetToolType_Scale;
	}

	KeyCode ScaleTool::GetShortcutKey() const
	{
		return KeyCode_R;
	}

	void ScaleTool::UpdatePostDrawGui(Graphics::Auth2D::Properties* properties, vec2 dimensions)
	{
		// TEMP:
		Gui::Text("scale: %.1f %.1f", properties->Scale.x, properties->Scale.y);

		ImGuiIO& io = Gui::GetIO();
		bool windowFocused = Gui::IsWindowFocused();
		bool windowHovered = Gui::IsWindowHovered();

		if (windowHovered && Gui::IsMouseClicked(0))
			windowFocused = true;

		vec2 mousePos = io.MousePos;
		vec2 mouseWorldPos = ToWorldSpace(mousePos);

		// if (mode == GrabMode::None)
		// 	worldSpaceBox = TransformBox(*properties, dimensions);
		// 
		// if (windowFocused || windowHovered)
		// 	screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

		if (windowFocused && Gui::IsMouseClicked(actionMouseButton))
		{
			for (int i = 0; i < ScaleNode_Count; i++)
			{
				if (glm::distance(screenSpaceNodes[i], mousePos) < NodeRadius)
					scalingNode = static_cast<ScaleNode>(i);
			}
		}

		hoveringNode = ScaleNode_Invalid;
		if (windowHovered)
		{
			for (int i = 0; i < ScaleNode_Count; i++)
			{
				if (glm::distance(screenSpaceNodes[i], mousePos) < NodeRadius)
					hoveringNode = static_cast<ScaleNode>(i);
			}
		}

		if (windowFocused && Gui::IsMouseClicked(actionMouseButton))
		{
			mouseWorldPositionOnMouseDown = mouseWorldPos;
			propertiesOnMouseDown = *properties;

			if (scalingNode >= 0)
				scaleNodeWorldPositionOnMouseDown = mouseWorldPos;
		}

		// NOTE: Make sure clicking on a node won't resize immediately until the mouse has been moved a bit
		if (!allowAction && Gui::IsMouseDragging(actionMouseButton, mouseDragThreshold))
			allowAction = true;

		if (!windowFocused || Gui::IsMouseReleased(actionMouseButton))
		{
			scalingNode = ScaleNode_Invalid;
			allowAction = false;
		}

		if (scalingNode >= 0)
		{
			Gui::SetMouseCursor(GetCursorForScaleNode(scalingNode));

			if (allowAction)
			{
				// MoveBoxCorner(scalingNode, worldSpaceBox, glm::round(mouseWorldPos), propertiesOnMouseDown.Rotation);
				// *properties = worldSpaceBox.GetProperties(dimensions, properties->Origin, properties->Rotation, properties->Opacity);
			}

			// DragScaleTooltip(properties->Scale, dimensions);
		}

		ImDrawList* drawList = Gui::GetWindowDrawList();

		// screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox);

		// base box
		// DrawBox(drawList, screenSpaceBox, (mode == GrabMode::Scale), mode == GrabMode::Move ? redColor : whiteColor);

		// origin center

		vec2 xPoisition = ToScreenSpace(GetAxisPoint(*properties, ScaleNode_AxisX));
		vec2 yPoisition = ToScreenSpace(GetAxisPoint(*properties, ScaleNode_AxisY));
		vec2 xyPoisition = ToScreenSpace(GetAxisPoint(*properties, ScaleNode_AxisXY));

		Gui::AddLine(drawList, xyPoisition, xPoisition, ImColor(xAxisColorLine));
		Gui::AddLine(drawList, xyPoisition, yPoisition, ImColor(yAxisColorLine));

		// TODO: Add rotated rectangle extension method (same as Renderer2D)
		//const float halfRadius = NodeCenterRadius * 0.5f;
		//drawList->AddQuadFilled(
		//	xyPoisition + vec2(-halfRadius, -halfRadius),
		//	xyPoisition + vec2(halfRadius, -halfRadius), 
		//	xyPoisition + vec2(halfRadius, halfRadius), 
		//	xyPoisition + vec2(-halfRadius, halfRadius), 
		//	ImColor(centerColor));

		Gui::AddQuadFilled(drawList, xyPoisition, vec2(NodeCenterRadius), vec2(NodeCenterRadius * 0.5f), properties->Rotation, vec2(1.0f), ImColor(centerColor));

		// drawList->AddCircleFilled(xyPoisition, NodeRadius, ImColor(centerColor));
		drawList->AddCircleFilled(xPoisition, NodeRadius, ImColor(xAxisColorInner));
		drawList->AddCircleFilled(yPoisition, NodeRadius, ImColor(yAxisColorInner));

		// grab node
		// if (mode == GrabMode::Scale)
		// 	drawList->AddCircle(screenSpaceBox.GetNodePosition(scalingNode), TransformBox::NodeRadius, ImColor(redColor));
	}

	void ScaleTool::DrawContextMenu()
	{
		// TODO: Scale to fixed values (?)
	}

	vec2 ScaleTool::GetAxisPoint(const Graphics::Auth2D::Properties& properties, ScaleNode node)
	{
		switch (node)
		{
		case ScaleNode_AxisX:
			return properties.Position + vec2(AxisLength, 0.0f);

		case ScaleNode_AxisY:
			return properties.Position + vec2(0.0f, -AxisLength);

		case ScaleNode_AxisXY:
			return properties.Position;
		}

		assert(false);
		return {};
	}
}