#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/Core/imgui.h"
#include "ImGui/Core/imgui_internal.h"
#include "ImGui/Core/imgui_markdown.h"
#include "ImGui/Extensions/Theme.h"
#include <functional>

namespace Comfy::Graphics
{
	class Tex;
	struct Spr;
	class SprSet;
}

#define BeginMenu WideBeginMenu

inline bool operator==(const ImRect& a, const ImRect& b) { return (a.Min.x == b.Min.x && a.Min.y == b.Min.y && a.Max.x == b.Max.x && a.Max.y == b.Max.y); }
inline bool operator!=(const ImRect& a, const ImRect& b) { return !(a == b); }

namespace ImGui
{
	constexpr vec2 UV0 = vec2(0.0f, 0.0f), UV1 = vec2(1.0f, 1.0f);
	constexpr vec2 UV0_R = vec2(0.0f, 1.0f), UV1_R = vec2(1.0f, 0.0f);

	constexpr const char* StringViewStart(std::string_view stringView) { return stringView.data(); }
	constexpr const char* StringViewEnd(std::string_view stringView) { return stringView.data() + stringView.size(); }

	ImRect FitFixedAspectRatio(ImRect sourceRegion, float targetAspectRatio);
	ImRect FitFixedAspectRatioImage(ImRect sourceRegion, vec2 imageDimensions);

	void UpdateExtendedState();

	bool WasActiveWindowFocusedOnMouseClicked(int button);
	bool WasActiveWindowHoveredOnMouseClicked(int button);

	bool WasHoveredWindowFocusedOnMouseClicked(int button);
	bool WasHoveredWindowHoveredOnMouseClicked(int button);

	void PushItemDisabledAndTextColorIf(bool condition);
	void PopItemDisabledAndTextColorIf(bool condition);

	void PushItemDisabledAndTextColor();
	void PopItemDisabledAndTextColor();

	void AddTexture(ImDrawList* drawList, const Comfy::Graphics::Tex* tex, vec2 center, vec2 scale, vec2 uv0, vec2 uv1);
	void AddSprite(ImDrawList* drawList, const Comfy::Graphics::Tex* tex, vec2 position, const vec4& sourceRegion, ImU32 color = IM_COL32_WHITE);
	void AddSprite(ImDrawList* drawList, const Comfy::Graphics::SprSet& sprSet, const Comfy::Graphics::Spr& spr, vec2 topLeft, vec2 bottomRight, ImU32 color);

	inline void AddTexture(ImDrawList* drawList, const Comfy::Graphics::Tex* tex, vec2 center, float scale, vec2 uv0, vec2 uv1)
	{
		AddTexture(drawList, tex, center, vec2(scale, scale), uv0, uv1);
	}

	inline void AddTexture(ImDrawList* drawList, const Comfy::Graphics::Tex* tex, vec2 center, float scale)
	{
		AddTexture(drawList, tex, center, vec2(scale, scale), UV0_R, UV1_R);
	}

	inline void AddDot(ImDrawList* drawList, vec2 position, ImU32 color)
	{
		vec2 bottomRight = position;
		++bottomRight.x; ++bottomRight.y;
		drawList->AddRectFilled(position, bottomRight, color);
	}

	inline void AddRectFilled(ImDrawList* drawList, const ImRect& rect, ImU32 color)
	{
		drawList->AddRectFilled(rect.GetTL(), rect.GetBR(), color);
	}

	void AddLine(ImDrawList* drawList, vec2 start, vec2 end, ImU32 color, float thickness = 1.0f);
	void AddQuadFilled(ImDrawList* drawList, vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, ImU32 color);

	bool IsItemHoveredDelayed(ImGuiHoveredFlags flags = ImGuiHoveredFlags_None, float threshold = 0.5f);

	bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	bool WideTreeNode(const char* label);
	bool WideTreeNode(const char* str_id, const char* fmt, ...);

	bool WideTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags);
	bool WideTreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);

	bool WideTreeNodeNoArrow(const char* label);
	bool WideTreeNodeNoArrow(const char* label, ImGuiTreeNodeFlags flags);

	bool WideBeginPopup(const char* label, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
	bool WideBeginMenu(const char* label, bool enabled = true);

	bool WideBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);

	bool WideCombo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

	void SetWideItemTooltip(const char* fmt, ...);
	void WideSetTooltip(const char* fmt, ...);
	void WideTooltip(const std::function<void(void)>& func);

	void HelpMarker(std::string_view description);
	void SameLineHelpMarker(std::string_view description);
	void SameLineHelpMarker(float localPosX, float spacingWidth, std::string_view description);
	void SameLineHelpMarkerRightAlign(std::string_view description);

	// TODO: Use templates instead of std::function wherever possible
	void WindowContextMenu(const char* str_id, const std::function<void(void)>& func);
	void ItemContextMenu(const char* str_id, const std::function<void(void)>& func);

	void ExtendedVerticalSeparator(float spacing = 8.0f);

	struct ExtendedImGuiTextFilter
	{
		ExtendedImGuiTextFilter(const char* default_filter = "");
		bool Draw(const char* label, const char* hint, float width = 0.0f);
		inline bool Draw(const char* label = "Filter (inc,-exc)", float width = 0.0f) { textFilter.Draw(label, width); }
		inline bool PassFilter(const char* text, const char* text_end = nullptr) { return textFilter.PassFilter(text, text_end); }
		inline void Build() { textFilter.Build(); }
		inline void Clear() { textFilter.Clear(); }
		inline bool IsActive() const { return !textFilter.IsActive(); }

	private:
		ImGuiTextFilter textFilter;
	};

	inline void DRAW_DEBUG_REGION(ImRect rect)
	{
		AddRectFilled(GetForegroundDrawList(), rect, static_cast<ImU32>(IM_COL32_BLACK * 0.5f));
	}

	template <typename Func>
	void DEBUG_NOSAVE_WINDOW(const char* windowName, Func function, ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize)
	{
		constexpr ImGuiWindowFlags defaultFlags = (ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking);
		if (Begin(windowName, nullptr, flags | defaultFlags))
			function();
		End();
	}

	template <typename Func>
	void DEBUG_NOSAVE_ONCE_PER_FRAME_WINDOW(const char* windowName, Func function, ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize)
	{
		constexpr ImGuiWindowFlags defaultFlags = (ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoDocking);
		if (Begin(windowName, nullptr, flags | defaultFlags))
		{
			int* lastFrame = GetStateStorage()->GetIntRef(GetID("last_frame"));
			int* thisFrame = GetStateStorage()->GetIntRef(GetID("this_frame"));

			*lastFrame = *thisFrame;
			*thisFrame = GetFrameCount();

			if (*thisFrame != *lastFrame)
				function();
		}
		End();
	}
}

#if COMFY_DEBUG || 1 // NOTE: For when typing 10 letters is too much for the task at hand

#define GUI_DEBUG_DRAG_FLOAT_WINDOW(inOutFloat) Gui::DEBUG_NOSAVE_ONCE_PER_FRAME_WINDOW(	\
	__FUNCTION__ "(l." COMFY_STRINGIFY(__LINE__) "): Debug Drag Float",							\
	[&] { Gui::DragFloat(COMFY_STRINGIFY(inOutFloat), &inOutFloat); })

#endif /* COMFY_DEBUG || 1 */
