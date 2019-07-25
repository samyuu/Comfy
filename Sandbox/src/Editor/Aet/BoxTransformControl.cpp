#include "BoxTransformControl.h"
#include "ImGui/imgui_extensions.h"
#include "AetIcons.h"

namespace Editor
{
	Box::Box()
	{
	}

	Box::Box(const Properties& properties, const vec2& dimensions)
	{
		if (properties.Rotation == 0.0f)
		{
			vec2 position = properties.Position + (properties.Origin * properties.Scale);
			vec2 size = dimensions * properties.Scale;

			TL = position;
			TR = vec2(position.x + size.x, position.y);
			BL = vec2(position.x, position.y + size.y);
			BR = position + size;
		}
		else
		{
			float radians = glm::radians(properties.Rotation);
			float sin = glm::sin(radians), cos = glm::cos(radians);

			vec2 origin = properties.Origin * properties.Scale;
			vec2 size = dimensions * properties.Scale;

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

	void BoxTransformControl::Draw(Properties* properties, vec2 dimensions, const std::function<void(vec2&)>& worldToScreenSpace, const std::function<void(vec2&)>& screenToWorldSpace, float zoom)
	{
		ImGuiIO& io = ImGui::GetIO();
		vec2 mousePos = io.MousePos;

		static int nodeIndex = -1;
		//if (ImGui::IsMouseClicked(0))
			nodeIndex = -1;

		Box box(*properties, dimensions);
		int i = 0;
		for (vec2* pos = &box.TL; pos <= &box.BR; pos++)
		{
			worldToScreenSpace(*pos);

			if (/*ImGui::IsMouseClicked(0) &&*/ glm::distance(*pos, mousePos) < BoxNodeRadius)
				nodeIndex = i;

			i++;
		}

		bool contains = box.Contains(mousePos);
		bool dragMoving = ImGui::IsMouseDown(0) && contains;

		//ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);

		if (dragMoving)
		{
			ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);

			if (vec2(io.MouseDelta) != vec2(0.0f) && ImGui::IsWindowHovered())
				properties->Position += vec2(io.MouseDelta) * (1.0f / zoom);

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(3.0f, 3.0f));
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
			{
				vec4 popupBg = ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
				popupBg.a *= .85f;
				ImGui::PushStyleColor(ImGuiCol_PopupBg, popupBg);
				{
					ImGui::BeginTooltip();
					{
						ImGui::Text(ICON_FA_ARROWS_ALT_H); ImGui::SameLine();
						ImGui::Text(":  %.f px", properties->Position.x);
						ImGui::Text(ICON_FA_ARROWS_ALT_V); ImGui::SameLine();
						ImGui::Text(":  %.f px", properties->Position.y);
					}
					ImGui::EndTooltip();
				}
				ImGui::PopStyleColor();
			}
			ImGui::PopStyleVar(2);
		}

		bool scaling = nodeIndex != -1;

		if (scaling)
		{
			ImGuiMouseCursor cursor;
			switch (nodeIndex)
			{
			case BoxNode_TL:
			case BoxNode_BR:
				cursor = ImGuiMouseCursor_ResizeNWSE;
				break;
			
			case BoxNode_TR:
			case BoxNode_BL:
				cursor = ImGuiMouseCursor_ResizeNESW;
				break;

			default:
				cursor = ImGuiMouseCursor_Hand;
				break;
			}

			ImGui::SetMouseCursor(cursor);

		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		DrawBox(drawList, box, scaling, contains ? vec4(1.0f, .25f, .25f, 1.0f) : vec4(1.0f));
	}
}