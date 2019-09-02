#include "BoxTransformControl.h"
#include "Editor/Aet/AetIcons.h"
#include "ImGui/Gui.h"

namespace Editor
{
	static inline void RotateVector(vec2& point, float sin, float cos)
	{
		point = vec2(point.x * cos - point.y * sin, point.x * sin + point.y * cos);
	}

	Box::Box()
	{
	}

	Box::Box(const Graphics::Auth2D::Properties& properties, const vec2& dimensions)
	{
		vec2 size = dimensions * properties.Scale;

		if (properties.Rotation == 0.0f)
		{
			vec2 position = properties.Position - (properties.Origin * properties.Scale);

			TL = position;
			TR = vec2(position.x + size.x, position.y);
			BL = vec2(position.x, position.y + size.y);
			BR = position + size;
		}
		else
		{
			float radians = glm::radians(properties.Rotation);
			float sin = glm::sin(radians), cos = glm::cos(radians);
			vec2 origin = -properties.Origin * properties.Scale;

			TL = properties.Position + vec2(origin.x * cos - origin.y * sin, origin.x * sin + origin.y * cos);
			TR = properties.Position + vec2((origin.x + size.x) * cos - origin.y * sin, (origin.x + size.x) * sin + origin.y * cos);
			BL = properties.Position + vec2(origin.x * cos - (origin.y + size.y) * sin, origin.x * sin + (origin.y + size.y) * cos);
			BR = properties.Position + vec2((origin.x + size.x) * cos - (origin.y + size.y) * sin, (origin.x + size.x) * sin + (origin.y + size.y) * cos);
		}
	}

	vec2 Box::Top() const
	{
		return (TL + TR) / 2.0f;
	}

	vec2 Box::Right() const
	{
		return (TR + BR) / 2.0f;
	}

	vec2 Box::Bottom() const
	{
		return (BL + BR) / 2.0f;
	}

	vec2 Box::Left() const
	{
		return (TL + BL) / 2.0f;
	}

	vec2 Box::GetNodePosition(int node)
	{
		switch (node)
		{
		case BoxNode_TL: return TL;
		case BoxNode_TR: return TR;
		case BoxNode_BL: return BL;
		case BoxNode_BR: return BR;
		case BoxNode_Top: return Top();
		case BoxNode_Right: return Right();
		case BoxNode_Bottom: return Bottom();
		case BoxNode_Left: return Left();
		}
		assert(false);
		return {};
	}

	Graphics::Auth2D::Properties Box::GetProperties(vec2 dimensions, vec2 origin, float rotation, float opacity) const
	{
		vec2 corners[2] = { TL, BR };
		vec2 rotationorigin = TL - origin;

		float radians = glm::radians(-rotation), sin = glm::sin(radians), cos = glm::cos(radians);
		for (vec2& corner : corners)
		{
			corner -= rotationorigin;
			RotateVector(corner, sin, cos);
			corner += rotationorigin;
		}

		vec2 scale = (corners[1] - corners[0]) / dimensions;
		vec2 originOffset = (origin * scale);
		RotateVector(originOffset, glm::sin(glm::radians(rotation)), glm::cos(glm::radians(rotation)));

		return
		{
			origin,
			TL + originOffset,
			rotation,
			scale,
			opacity,
		};
	}

	bool Box::Contains(const vec2& point) const
	{
		vec2 e = vec2(TR.x - TL.x, TR.y - TL.y);
		vec2 f = vec2(BL.x - TL.x, BL.y - TL.y);

		return !(
			((point.x - TL.x) * e.x + (point.y - TL.y) * e.y < 0.0) ||
			((point.x - TR.x) * e.x + (point.y - TR.y) * e.y > 0.0) ||
			((point.x - TL.x) * f.x + (point.y - TL.y) * f.y < 0.0) ||
			((point.x - BL.x) * f.x + (point.y - BL.y) * f.y > 0.0));
	}

	constexpr float BoxNodeRadius = 4.0f;

	static void DrawBox(ImDrawList* drawList, const Box& box, bool cross, const vec4& color)
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

		drawList->AddCircleFilled(box.TL, BoxNodeRadius, colorU32);
		drawList->AddCircleFilled(box.TR, BoxNodeRadius, colorU32);
		drawList->AddCircleFilled(box.BL, BoxNodeRadius, colorU32);
		drawList->AddCircleFilled(box.BR, BoxNodeRadius, colorU32);

		drawList->AddCircleFilled(box.Top(), BoxNodeRadius, colorU32);
		drawList->AddCircleFilled(box.Right(), BoxNodeRadius, colorU32);
		drawList->AddCircleFilled(box.Bottom(), BoxNodeRadius, colorU32);
		drawList->AddCircleFilled(box.Left(), BoxNodeRadius, colorU32);
	}

	static Box BoxWorldToScreenSpace(const Box& box, const std::function<vec2(vec2)>& worldToScreenSpace)
	{
		Box screenSpaceBox;
		for (int i = 0; i < sizeof(box.Corners) / sizeof(vec2); i++)
			screenSpaceBox.Corners[i] = worldToScreenSpace(box.Corners[i]);
		return screenSpaceBox;
	}

	static ImGuiMouseCursor GetCursorForBoxNode(int boxNode)
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
			return 0;
		}
	}

	void BoxTransformControl::Draw(Graphics::Auth2D::Properties* properties, vec2 dimensions, const std::function<vec2(vec2)>& worldToScreenSpace, const std::function<vec2(vec2)>& screenToWorldSpace, float zoom)
	{
		constexpr vec4 redColor = vec4(1.0f, .25f, .25f, 0.85f);
		constexpr vec4 yellowColor = vec4(.75f, .75f, .25f, 0.85f);
		constexpr vec4 whiteColor = vec4(1.0f, 1.0f, 1.0f, 0.85f);

		ImGuiIO& io = Gui::GetIO();
		bool windowFocused = Gui::IsWindowFocused();
		bool windowHovered = Gui::IsWindowHovered();

		vec2 mousePos = io.MousePos;
		vec2 mouseWorldPos = screenToWorldSpace(mousePos);

		if (mode == GrabMode::None)
			worldSpaceBox = Box(*properties, dimensions);

		if (windowFocused || windowHovered)
			screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox, worldToScreenSpace);

		if (windowFocused && Gui::IsMouseClicked(0))
		{
			for (int i = 0; i < BoxNode_Count; i++)
			{
				if (glm::distance(screenSpaceBox.GetNodePosition(i), mousePos) < BoxNodeRadius)
					scalingNode = i;
			}
		}

		hoveringNode = -1;
		if (windowHovered)
		{
			for (int i = 0; i < BoxNode_Count; i++)
			{
				if (glm::distance(screenSpaceBox.GetNodePosition(i), mousePos) < BoxNodeRadius)
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

			// TODO: round
			vec2 grabOffset = mouseWorldPositionOnMouseDown - propertiesOnMouseDown.Position;
			properties->Position = glm::round(mouseWorldPos - grabOffset);

			worldSpaceBox = Box(*properties, dimensions);

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
			Box worldBoxOnMouseDown = Box(propertiesOnMouseDown, dimensions);
			Box screenBoxOnMouseDown = BoxWorldToScreenSpace(worldBoxOnMouseDown, worldToScreenSpace);
			DrawBox(drawList, screenBoxOnMouseDown, false, vec4(1.0f, 1.0f, 1.0f, 0.25f));
		}

		screenSpaceBox = BoxWorldToScreenSpace(worldSpaceBox, worldToScreenSpace);

		// base box
		DrawBox(drawList, screenSpaceBox, (mode == GrabMode::Scale), mode == GrabMode::Move ? redColor : whiteColor);

		// origin center
		drawList->AddCircleFilled(worldToScreenSpace(properties->Position), BoxNodeRadius, ImColor(yellowColor));

		// grab node
		if (mode == GrabMode::Scale)
			drawList->AddCircle(screenSpaceBox.GetNodePosition(scalingNode), BoxNodeRadius, ImColor(redColor));
	}

	void BoxTransformControl::MoveBoxCorner(Box& box, vec2 position, float rotation)
	{
		// vec2 origin = box.TL;
		vec2 origin = vec2();

		if (rotation != 0.0f)
		{
			float radians = glm::radians(-rotation), sin = glm::sin(radians), cos = glm::cos(radians);

			for (auto& corner : worldSpaceBox.Corners)
			{
				corner -= origin;
				RotateVector(corner, sin, cos);
			}

			position -= origin;
			RotateVector(position, sin, cos);
		}

		if (scalingNode >= BoxNode_TL && scalingNode <= BoxNode_BR)
		{
			int cornerNodes[2][4] = { { BoxNode_BL, BoxNode_BR, BoxNode_TL, BoxNode_TR }, { BoxNode_TR, BoxNode_TL, BoxNode_BR, BoxNode_BL } };

			worldSpaceBox.Corners[scalingNode] = position;
			for (int i = 0; i < 2; i++)
				worldSpaceBox.Corners[cornerNodes[i][scalingNode]][i] = position[i];
		}
		else
		{
			int edgeNodes[2][4] = { { BoxNode_TL, BoxNode_TR, BoxNode_BL, BoxNode_TL }, { BoxNode_TR, BoxNode_BR, BoxNode_BR, BoxNode_BL } };

			for (int i = 0; i < 2; i++)
				worldSpaceBox.Corners[edgeNodes[i][scalingNode - BoxNode_Top]][~scalingNode & 1] = position[~scalingNode & 1];
		}

		if (rotation != 0.0f)
		{
			float radians = glm::radians(rotation), sin = glm::sin(radians), cos = glm::cos(radians);

			for (auto& corner : worldSpaceBox.Corners)
			{
				RotateVector(corner, sin, cos);
				corner += origin;
			}
		}
	}

	void BoxTransformControl::DragPositionTooltip(const vec2& position)
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

	void BoxTransformControl::DragScaleTooltip(const vec2& scale, const vec2& dimensions)
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
}