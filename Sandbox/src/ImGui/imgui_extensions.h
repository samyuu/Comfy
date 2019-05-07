#pragma once
#include "imgui.h"
#include "imgui_internal.h"

class Texture;

namespace ImGui
{
	void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0 = ImVec2(0, 1), const ImVec2& uv1 = ImVec2(1, 0));

	inline void AddTexture(ImDrawList* drawList, Texture* texture, ImVec2 center, float scale)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale));
	};

	inline void AddDot(ImDrawList* drawList, ImVec2 position, ImU32 color)
	{
		ImVec2 bottomRight = position;
		++bottomRight.x; ++bottomRight.y;
		drawList->AddRectFilled(position, bottomRight, color);
	}
}