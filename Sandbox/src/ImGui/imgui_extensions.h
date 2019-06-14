#pragma once
#include "imgui.h"
#include "imgui_internal.h"

class Texture;

namespace ImGui
{
	void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0));

	inline void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, float scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale), uv0, uv1);
	};

	inline void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, float scale)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale));
	};

	inline void AddDot(ImDrawList* drawList, const ImVec2& position, ImU32 color)
	{
		ImVec2 bottomRight = position;
		++bottomRight.x; ++bottomRight.y;
		drawList->AddRectFilled(position, bottomRight, color);
	}

	inline void AddRectFilled(ImDrawList* drawList, const ImRect& rect, ImU32 color)
	{
		drawList->AddRectFilled(rect.GetTL(), rect.GetBR(), color);
	}

	void StyleComfy(ImGuiStyle* dst = nullptr);

	inline void DRAW_DEBUG_REGION(ImRect& rect) 
	{ 
		ImGui::AddRectFilled(ImGui::GetForegroundDrawList(), rect, static_cast<ImU32>(IM_COL32_BLACK * .5f)); 
	};
}