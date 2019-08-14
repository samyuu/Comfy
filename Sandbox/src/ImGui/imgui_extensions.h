#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_internal.h"
#include "theme.h"
#include <functional>

namespace Graphics
{
	class Texture2D;
}

#define BeginMenu WideBeginMenu

namespace ImGui
{
	const ImVec2 UV0 = ImVec2(0, 0), UV1 = ImVec2(1, 1);
	const ImVec2 UV0_GL = ImVec2(0, 1), UV1_GL = ImVec2(1, 0);

	void UpdateExtendedState();

	bool WasActiveWindowFocusedOnMouseClicked(int button);
	bool WasActiveWindowHoveredOnMouseClicked(int button);

	bool WasHoveredWindowFocusedOnMouseClicked(int button);
	bool WasHoveredWindowHoveredOnMouseClicked(int button);

	void AddTexture(ImDrawList* drawList, Graphics::Texture2D* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0 = UV0_GL, const ImVec2& uv1 = UV1_GL);

	inline void AddTexture(ImDrawList* drawList, Graphics::Texture2D* texture, ImVec2 center, float scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale), uv0, uv1);
	};

	inline void AddTexture(ImDrawList* drawList, Graphics::Texture2D* texture, ImVec2 center, float scale)
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

	bool WideBeginPopup(const char* label);
	bool WideBeginMenu(const char* label, bool enabled = true);

	bool WideBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);

	bool WideCombo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

	void WideSetTooltip(const char* fmt, ...);

	void WindowContextMenu(const char* str_id, const std::function<void(void)>& func);
	void ItemContextMenu(const char* str_id, const std::function<void(void)>& func);

	bool ExtendedInputFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, bool disabled = false);

	void ExtendedVerticalSeparator(float spacing = 8.0f);

	struct ExtendedImGuiTextFilter
	{
		ExtendedImGuiTextFilter(const char* default_filter = "");
		bool Draw(const char* label, const char* hint, float width = 0.0f);
		inline bool Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f) { textFilter.Draw(label, width); };
		inline bool PassFilter(const char* text, const char* text_end = NULL) { return textFilter.PassFilter(text, text_end); };
		inline void Build() { textFilter.Build(); };
		inline void Clear() { textFilter.Clear(); }
		inline bool IsActive() const { return !textFilter.IsActive(); }

	private:
		ImGuiTextFilter textFilter;
	};

	inline void DRAW_DEBUG_REGION(ImRect& rect)
	{
		ImGui::AddRectFilled(ImGui::GetForegroundDrawList(), rect, static_cast<ImU32>(IM_COL32_BLACK * .5f));
	};
}