#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/Core/imgui.h"
#include "ImGui/Core/imgui_internal.h"
#include "ImGui/Extensions/Theme.h"
#include "Time/TimeSpan.h"
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

	f32 AnimateFadeInOutOpacity(f32& inOutCurrentOpacity, bool isFadingIn, f32 fadeInDurationSec = 0.5f, f32 fadeOutDurationSec = 0.5f, f32 minOpacity = 0.0f, f32 maxOpacity = 1.0f);
	f32 AnimateFadeInOutOpacity(f32& inOutCurrentOpacity, bool isFadingIn, Comfy::TimeSpan fadeInDuration, Comfy::TimeSpan fadeOutDuration, f32 minOpacity = 0.0f, f32 maxOpacity = 1.0f);
	f32 AnimateFadeInOutOpacity(f32& inOutCurrentOpacity, bool isFadingIn, Comfy::TimeSpan fadeInOutDuration, f32 minOpacity = 0.0f, f32 maxOpacity = 1.0f);

	struct TooltipFadeInOutHelper
	{
		const void* CurrentID = {};
		f32 CurrentOpacity = {};
		Comfy::TimeSpan CurrentElapsedHoverTime = {};

		// NOTE: Small delay to prevent accidentally showing the tooltip when quickly moving the mouse over and past the help marker
		Comfy::TimeSpan FadeInDelay = Comfy::TimeSpan::FromMilliseconds(60.0);
		Comfy::TimeSpan FadeInDuration = Comfy::TimeSpan::FromMilliseconds(120.0);
		Comfy::TimeSpan FadeOutDuration = Comfy::TimeSpan::FromMilliseconds(120.0);
		f32 MinOpacity = 0.0f;
		f32 MaxOpacity = 1.0f;

		void ResetFade(const void* newID = nullptr);
		f32 UpdateFadeAndGetOpacity(const void* newID, bool isItemHovered);
	};

	void UpdateExtendedState();

	bool WasActiveWindowFocusedOnMouseClicked(int button);
	bool WasActiveWindowHoveredOnMouseClicked(int button);

	bool WasHoveredWindowFocusedOnMouseClicked(int button);
	bool WasHoveredWindowHoveredOnMouseClicked(int button);

	void PushItemDisabledAndTextColorIf(bool condition);
	void PopItemDisabledAndTextColorIf(bool condition);

	void PushItemDisabledAndTextColor();
	void PopItemDisabledAndTextColor();

	void AddTextWithShadow(ImDrawList* drawList, vec2 position, std::string_view text, u32 color = GetColorU32(ImGuiCol_Text), u32 shadowColor = 0xFF000000, vec2 shadowOffset = vec2(1.0f));

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
	bool PathInputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0);

	bool InputFormattedTimeSpan(const char* label, Comfy::TimeSpan* value, vec2 size = {}, ImGuiInputTextFlags flags = 0);

	bool WideTreeNode(const char* label);
	bool WideTreeNode(const char* str_id, const char* fmt, ...);

	bool WideTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags);
	bool WideTreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...);

	bool WideTreeNodeNoArrow(const char* label);
	bool WideTreeNodeNoArrow(const char* label, ImGuiTreeNodeFlags flags);

	bool WideBeginPopup(const char* label, ImGuiWindowFlags flags = ImGuiWindowFlags_None);
	bool WideBeginPopupModal(const char* name, bool* p_open = nullptr, ImGuiWindowFlags flags = 0);
	bool WideBeginMenu(const char* label, bool enabled = true);

	bool WideBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);

	bool WideCombo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	bool WideCombo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);

	bool MenuItemDontClosePopup(const char* label, const char* shortcut = nullptr, bool selected = false, bool enabled = true);
	bool MenuItemDontClosePopup(const char* label, const char* shortcut, bool* selected, bool enabled = true);

	void SetWideItemTooltip(const char* fmt, ...);
	void WideSetTooltip(const char* fmt, ...);
	void WideSetTooltip(vec2 position, vec2 pivot, const char* fmt, ...);
	void WideTooltip(const std::function<void(void)>& func);
	void WideTooltip(vec2 position, vec2 pivot, const std::function<void(void)>& func);

	void HelpMarker(std::string_view description);
	void SameLineHelpMarker(std::string_view description);
	void SameLineHelpMarker(float localPosX, float spacingWidth, std::string_view description);
	void SameLineHelpMarkerRightAlign(std::string_view description);

	bool IsMouseSteady();

	// TODO: Use templates instead of std::function wherever possible
	void WindowContextMenu(const char* str_id, const std::function<void(void)>& func);
	void ItemContextMenu(const char* str_id, const std::function<void(void)>& func);

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
