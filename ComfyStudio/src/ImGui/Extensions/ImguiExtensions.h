#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/Core/imgui.h"
#include "ImGui/Core/imgui_internal.h"
#include "ImGui/Extensions/Theme.h"
#include <functional>

namespace Graphics
{
	class D3D_Texture2D;
}

#define BeginMenu WideBeginMenu

namespace ImGui
{
	const ImVec2 UV0 = ImVec2(0.0f, 0.0f), UV1 = ImVec2(1.0f, 1.0f);

	void UpdateExtendedState();

	bool WasActiveWindowFocusedOnMouseClicked(int button);
	bool WasActiveWindowHoveredOnMouseClicked(int button);

	bool WasHoveredWindowFocusedOnMouseClicked(int button);
	bool WasHoveredWindowHoveredOnMouseClicked(int button);

	void PushItemDisabledAndTextColorIf(bool condition);
	void PopItemDisabledAndTextColorIf(bool condition);

	void PushItemDisabledAndTextColor();
	void PopItemDisabledAndTextColor();

	void AddTexture(ImDrawList* drawList, const Graphics::D3D_Texture2D* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0 = UV0, const ImVec2& uv1 = UV1);
	void AddSprite(ImDrawList* drawList, const Graphics::D3D_Texture2D* texture, const vec2& position, const vec4& sourceRegion, ImU32 color = IM_COL32_WHITE);

	inline void AddTexture(ImDrawList* drawList, const Graphics::D3D_Texture2D* texture, ImVec2 center, float scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale), uv0, uv1);
	}

	inline void AddTexture(ImDrawList* drawList, const Graphics::D3D_Texture2D* texture, ImVec2 center, float scale)
	{
		AddTexture(drawList, texture, center, ImVec2(scale, scale));
	}

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

	void AddLine(ImDrawList* drawList, vec2 start, vec2 end, ImU32 color, float thickness = 1.0f);
	void AddQuadFilled(ImDrawList* drawList, vec2 position, vec2 size, vec2 origin, float rotation, const vec2& scale, ImU32 color);
	
	bool IsItemHoveredDelayed(ImGuiHoveredFlags flags = ImGuiHoveredFlags_None, float threshold = 0.5f);

	bool WideTreeNode(const char* label);
	bool WideTreeNode(const char* str_id, const char* fmt, ...);

	bool WideTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags);
	bool WideTreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);

	bool WideTreeNodeNoArrow(const char* label);
	bool WideTreeNodeNoArrow(const char* label, ImGuiTreeNodeFlags flags);

	bool WideBeginPopup(const char* label);
	bool WideBeginMenu(const char* label, bool enabled = true);

	bool WideBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);

	bool WideCombo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

	void SetWideItemTooltip(const char* fmt, ...);
	void WideSetTooltip(const char* fmt, ...);
	void WideTooltip(const std::function<void(void)>& func);

	void WindowContextMenu(const char* str_id, const std::function<void(void)>& func);
	void ItemContextMenu(const char* str_id, const std::function<void(void)>& func);

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
		AddRectFilled(GetForegroundDrawList(), rect, static_cast<ImU32>(IM_COL32_BLACK * 0.5f));
	}

	inline void DEBUG_NOSAVE_WINDOW(const char* windowName, const std::function<void(void)>& function, ImGuiWindowFlags flags = 0)
	{
		constexpr ImGuiWindowFlags defaultFlags = (ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking);
		if (Begin(windowName, nullptr, flags | defaultFlags))
			function();
		End();
	}
}