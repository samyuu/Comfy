#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include <functional>

class Texture2D;

namespace ImGui
{
	const ImVec2 UV0 = ImVec2(0, 0), UV1 = ImVec2(1, 1);
	const ImVec2 UV0_GL = ImVec2(0, 1), UV1_GL = ImVec2(1, 0);

	void UpdateExtendedState();

	bool WasActiveWindowFocusedOnMouseClicked(int button);
	bool WasActiveWindowHoveredOnMouseClicked(int button);
	
	bool WasHoveredWindowFocusedOnMouseClicked(int button);
	bool WasHoveredWindowHoveredOnMouseClicked(int button);

	void AddTexture(ImDrawList* drawList, Texture2D* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0 = UV0_GL, const ImVec2& uv1 = UV1_GL);

	inline void AddTexture(ImDrawList* drawList, Texture2D* texture, ImVec2 center, float scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale), uv0, uv1);
	};

	inline void AddTexture(ImDrawList* drawList, Texture2D* texture, ImVec2 center, float scale)
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

	bool IsItemHoveredDelayed(ImGuiHoveredFlags flags = ImGuiHoveredFlags_None, float threshold = .5f);

	void HelpMarker(const char* description);
	void SameLineHelpMarker(const char* description);

	bool WideTreeNode(const char* label);
	bool WideTreeNode(const char* str_id, const char* fmt, ...);

	bool WideTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags);
	bool WideTreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);

	bool WideTreeNodeNoArrow(const char* label);
	bool WideTreeNodeNoArrow(const char* label, ImGuiTreeNodeFlags flags);
	bool SmallButton(const char* label, const ImVec2& size);

	void WindowContextMenu(const char* std_id, const std::function<void(void)>& func);
	void ItemContextMenu(const char* std_id, const std::function<void(void)>& func);

	bool ExtendedInputFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, bool disabled = false);

	void ExtendedVerticalSeparator(float spacing = 8.0f);

	inline void DRAW_DEBUG_REGION(ImRect& rect)
	{
		ImGui::AddRectFilled(ImGui::GetForegroundDrawList(), rect, static_cast<ImU32>(IM_COL32_BLACK * .5f));
	};
}