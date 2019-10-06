#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImguiExtensions.h"
#include "Graphics/Texture/Texture2D.h"
#include "Core/DebugStopwatch.h"
#include "FontIcons.h"

namespace ImGui
{
	static ImGuiWindow* activeWindowsOnMouseClicks[5];
	static ImGuiWindow* hoveredWindowsOnMouseClicks[5];

	struct RAII_PopupWindowPadding
	{
		RAII_PopupWindowPadding() { PushStyleVar(ImGuiStyleVar_WindowPadding, PopupWindowPadding); }
		~RAII_PopupWindowPadding() { PopStyleVar(); }
	};

#define RAII_POPUP_WINDOW_PADDING() RAII_PopupWindowPadding uniquename(__RAII_POPUP_WINDOW_PADDING)

	void UpdateExtendedState()
	{
		for (int i = 0; i < 5; i++)
		{
			if (IsMouseClicked(i, false))
			{
				activeWindowsOnMouseClicks[i] = GImGui->NavWindow;
				hoveredWindowsOnMouseClicks[i] = GImGui->HoveredWindow;
			}
		}
	}

	bool WasActiveWindowFocusedOnMouseClicked(int button)
	{
		IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(GImGui->IO.MouseDown));
		return GImGui->NavWindow == activeWindowsOnMouseClicks[button];
	}

	bool WasActiveWindowHoveredOnMouseClicked(int button)
	{
		IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(GImGui->IO.MouseDown));
		return GImGui->NavWindow == hoveredWindowsOnMouseClicks[button];
	}

	bool WasHoveredWindowFocusedOnMouseClicked(int button)
	{
		IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(GImGui->IO.MouseDown));
		return GImGui->HoveredWindow == activeWindowsOnMouseClicks[button];
	}

	bool WasHoveredWindowHoveredOnMouseClicked(int button)
	{
		IM_ASSERT(button >= 0 && button < IM_ARRAYSIZE(GImGui->IO.MouseDown));
		return GImGui->HoveredWindow == hoveredWindowsOnMouseClicks[button];
	}

	void AddTexture(ImDrawList* drawList, const Graphics::Texture2D* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		float width = texture->GetWidth() * scale.x;
		float height = texture->GetHeight() * scale.y;

		center.x -= width * .5f;
		center.y -= height * .5f;
		ImVec2 bottomRight(center.x + width, center.y + height);

		drawList->AddImage(texture->GetVoidTexture(), center, bottomRight, uv0, uv1);
	}

	void AddSprite(ImDrawList* drawList, const Graphics::Texture2D* texture, const vec2& position, const vec4& sourceRegion, ImU32 color)
	{
		const vec2 textureSize = texture->GetSize();

		vec2 uv0, uv1;
		uv0.x = (sourceRegion.x / textureSize.x);
		uv1.x = uv0.x + (sourceRegion.z / textureSize.x);

		uv0.y = 1.0f - (sourceRegion.y / textureSize.y);
		uv1.y = uv0.y + (sourceRegion.w / textureSize.y);

		drawList->AddImage(texture->GetVoidTexture(), position, position + vec2(sourceRegion.z, sourceRegion.w), uv0, uv1, color);
	}

	void AddLine(ImDrawList* drawList, vec2 start, vec2 end, ImU32 color, float thickness)
	{
		drawList->AddLine(glm::round(start), glm::round(end), color, thickness);
	}

	void AddQuadFilled(ImDrawList * drawList, vec2 position, vec2 size, vec2 origin, float rotation, const vec2 & scale, ImU32 color)
	{
		size *= scale;
		origin *= -scale;

		vec2 topLeft, topRight, bottomLeft, bottomRight;

		if (rotation == 0.0f)
		{
			position += origin;

			topLeft = position;
			topRight = position + vec2(size.x, 0.0f);
			bottomLeft = position + vec2(0.0f, size.y);
			bottomRight = position + size;
		}
		else
		{
			const float radians = glm::radians(rotation);
			const float sin = glm::sin(radians);
			const float cos = glm::cos(radians);

			topLeft.x = position.x + origin.x * cos - origin.y * sin;
			topLeft.y = position.y + origin.x * sin + origin.y * cos;

			topRight.x = position.x + (origin.x + size.x) * cos - origin.y * sin;
			topRight.y = position.y + (origin.x + size.x) * sin + origin.y * cos;

			bottomLeft.x = position.x + origin.x * cos - (origin.y + size.y) * sin;
			bottomLeft.y = position.y + origin.x * sin + (origin.y + size.y) * cos;

			bottomRight.x = position.x + (origin.x + size.x) * cos - (origin.y + size.y) * sin;
			bottomRight.y = position.y + (origin.x + size.x) * sin + (origin.y + size.y) * cos;
		}

		drawList->AddQuadFilled(glm::round(topLeft), glm::round(topRight), glm::round(bottomRight), glm::round(bottomLeft), color);
	}

	bool IsItemHoveredDelayed(ImGuiHoveredFlags flags, float threshold)
	{
		return IsItemHovered(flags) && GImGui->HoveredIdTimer > threshold;
	}

	bool WideTreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, bool no_arrow = false)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
		const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, 0.0f);

		if (!label_end)
			label_end = FindRenderedTextEnd(label);
		const ImVec2 label_size = CalcTextSize(label, label_end, false);

		// We vertically grow up to current line height up the typical widget height.
		float text_base_offset_y = ImMax(padding.y, window->DC.CurrentLineTextBaseOffset); // Latch before ItemSize changes it
		const float frame_height = ImMax(ImMin(window->DC.CurrentLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
		ImRect frame_bb = ImRect(ImVec2(window->Pos.x, window->DC.CursorPos.y), ImVec2(window->Pos.x + GetContentRegionMax().x, window->DC.CursorPos.y + frame_height));

		// Selectables are tightly packed together so we extend the box to cover spacing between selectable.
		{
			const float spacing_x = style.ItemSpacing.x;
			const float spacing_y = style.ItemSpacing.y;
			const float spacing_L = (float)(int)(spacing_x * 0.50f);
			const float spacing_U = (float)(int)(spacing_y * 0.50f);
			frame_bb.Min.x -= spacing_L;
			frame_bb.Min.y -= spacing_U;
			frame_bb.Max.x += (spacing_x - spacing_L);
			frame_bb.Max.y += (spacing_y - spacing_U);

			text_base_offset_y += spacing_U;
		}

		if (display_frame)
		{
			// Framed header expand a little outside the default padding
			frame_bb.Min.x -= (float)(int)(window->WindowPadding.x*0.5f) - 1;
			frame_bb.Max.x += (float)(int)(window->WindowPadding.x*0.5f) - 1;
		}

		//ImRect indended_frame_bb = frame_bb;
		//indended_frame_bb.Min.x -= window->Pos.x - window->DC.CursorPos.x;
		ImRect indended_frame_bb(frame_bb.Min.x - (window->Pos.x - window->DC.CursorPos.x), frame_bb.Min.y, frame_bb.Max.x, frame_bb.Max.y);

		const float text_offset_x = no_arrow ? padding.x * 2 : (g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2));   // Collapser arrow width + Spacing
		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);   // Include collapser
		ItemSize(ImVec2(text_width, frame_height), text_base_offset_y);

		// Always use the hit test up to the right side of the content
		//const ImRect interact_bb = ImRect(frame_bb.Min.x, frame_bb.Min.y, frame_bb.Min.x + text_width + style.ItemSpacing.x * 2, frame_bb.Max.y);
		const ImRect interact_bb = frame_bb;
		bool is_open = TreeNodeBehaviorIsOpen(id, flags);
		bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;

		// Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
		// For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
		// This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
		if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			window->DC.TreeDepthMayJumpToParentOnPop |= (1 << window->DC.TreeDepth);

		bool item_add = ItemAdd(interact_bb, id);
		window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
		window->DC.LastItemDisplayRect = frame_bb;

		if (!item_add)
		{
			if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
				TreePushRawID(id);
			IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
			return is_open;
		}

		// Flags that affects opening behavior:
		// - 0 (default) .................... single-click anywhere to open
		// - OpenOnDoubleClick .............. double-click anywhere to open
		// - OpenOnArrow .................... single-click on arrow to open
		// - OpenOnDoubleClick|OpenOnArrow .. single-click on arrow or double-click anywhere to open
		ImGuiButtonFlags button_flags = ImGuiButtonFlags_NoKeyModifiers;
		if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
			button_flags |= ImGuiButtonFlags_AllowItemOverlap;
		if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
			button_flags |= ImGuiButtonFlags_PressedOnDoubleClick | ((flags & ImGuiTreeNodeFlags_OpenOnArrow) ? ImGuiButtonFlags_PressedOnClickRelease : 0);
		if (!is_leaf)
			button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

		bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
		const bool was_selected = selected;

		bool hovered, held;
		bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
		bool toggled = false;
		if (!is_leaf)
		{
			if (pressed)
			{
				toggled = !(flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) || (g.NavActivateId == id);
				if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
					toggled |= IsMouseHoveringRect(indended_frame_bb.Min, ImVec2(indended_frame_bb.Min.x + text_offset_x, indended_frame_bb.Max.y)) && (!g.NavDisableMouseHover);
				if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
					toggled |= g.IO.MouseDoubleClicked[0];
				if (g.DragDropActive && is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
					toggled = false;
			}

			if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Left && is_open)
			{
				toggled = true;
				NavMoveRequestCancel();
			}
			if (g.NavId == id && g.NavMoveRequest && g.NavMoveDir == ImGuiDir_Right && !is_open) // If there's something upcoming on the line we may want to give it the priority?
			{
				toggled = true;
				NavMoveRequestCancel();
			}

			if (toggled)
			{
				is_open = !is_open;
				window->DC.StateStorage->SetInt(id, is_open);
			}
		}
		if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
			SetItemAllowOverlap();

		// In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
			window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
		const ImVec2 text_pos = indended_frame_bb.Min + ImVec2(text_offset_x, text_base_offset_y);
		ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
		if (display_frame)
		{
			// Framed type
			RenderFrame(frame_bb.Min, frame_bb.Max, col, true, style.FrameRounding);
			RenderNavHighlight(frame_bb, id, nav_highlight_flags);
			if (!no_arrow)
				RenderArrow(indended_frame_bb.Min + ImVec2(padding.x, text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
			if (g.LogEnabled)
			{
				// NB: '##' is normally used to hide text (as a library-wide feature), so we need to specify the text range to make sure the ## aren't stripped out here.
				const char log_prefix[] = "\n##";
				const char log_suffix[] = "##";
				LogRenderedText(&text_pos, log_prefix, log_prefix + 3);
				RenderTextClipped(text_pos, indended_frame_bb.Max, label, label_end, &label_size);
				LogRenderedText(&text_pos, log_suffix, log_suffix + 2);
			}
			else
			{
				RenderTextClipped(text_pos, indended_frame_bb.Max, label, label_end, &label_size);
			}
		}
		else
		{
			// Unframed typed for tree nodes
			if (hovered || selected)
			{
				RenderFrame(frame_bb.Min, frame_bb.Max, col, false);
				RenderNavHighlight(frame_bb, id, nav_highlight_flags);
			}

			if (!no_arrow)
			{
				if (flags & ImGuiTreeNodeFlags_Bullet)
					RenderBullet(indended_frame_bb.Min + ImVec2(text_offset_x * 0.5f, g.FontSize*0.50f + text_base_offset_y));
				else if (!is_leaf)
					RenderArrow(indended_frame_bb.Min + ImVec2(padding.x, g.FontSize*0.15f + text_base_offset_y), is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
			}
			if (g.LogEnabled)
				LogRenderedText(&text_pos, ">");
			RenderText(text_pos, label, label_end, false);
		}

		if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			TreePushRawID(id);
		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.ItemFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
		return is_open;
	}

	bool WideTreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
		return WideTreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
	}

	bool WideTreeNode(const char* label)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), 0, label, NULL);
	}

	bool WideTreeNode(const char* str_id, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		bool is_open = WideTreeNodeExV(str_id, 0, fmt, args);
		va_end(args);
		return is_open;
	}

	bool WideTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), flags, label, NULL);
	}

	bool WideTreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		bool is_open = WideTreeNodeExV(ptr_id, flags, fmt, args);
		va_end(args);
		return is_open;
	}

	bool WideTreeNodeNoArrow(const char* label)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), 0, label, NULL, true);
	}

	bool WideTreeNodeNoArrow(const char* label, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), flags, label, NULL, true);
	}

	bool WideBeginPopup(const char* label)
	{
		RAII_POPUP_WINDOW_PADDING();
		return BeginPopup(label);
	}

	bool WideBeginMenu(const char* label, bool enabled)
	{
		RAII_POPUP_WINDOW_PADDING();
#undef BeginMenu
		return BeginMenu(label, enabled);
	}

	bool WideBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags)
	{
		RAII_POPUP_WINDOW_PADDING();
		return BeginCombo(label, preview_value, flags);
	}

	bool WideCombo(const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items)
	{
		RAII_POPUP_WINDOW_PADDING();
		return Combo(label, current_item, items, items_count, popup_max_height_in_items);
	}

	bool WideCombo(const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items)
	{
		RAII_POPUP_WINDOW_PADDING();
		return Combo(label, current_item, items_separated_by_zeros, popup_max_height_in_items);
	}

	bool WideCombo(const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items)
	{
		RAII_POPUP_WINDOW_PADDING();
		return Combo(label, current_item, items_getter, data, items_count, popup_max_height_in_items);
	}

	void WideSetTooltip(const char* fmt, ...)
	{
		RAII_POPUP_WINDOW_PADDING();

		va_list args;
		va_start(args, fmt);
		SetTooltipV(fmt, args);
		va_end(args);
	}

	void WideTooltip(const std::function<void(void)>& func)
	{
		RAII_POPUP_WINDOW_PADDING();
		BeginTooltip();
		func();
		EndTooltip();
	}

	constexpr int ContextMenuMouseButton_button = 1;

	static bool GetIsMouseSteady()
	{
		constexpr float threshold = 2.0f;

		vec2 mouseDragDelta = GetMouseDragDelta(1);
		return fabs(mouseDragDelta.x) < threshold && fabs(mouseDragDelta.y) < threshold;
	}

	static bool InternalBeginContextMenu(const char* str_id, bool checkItemHover)
	{
		RAII_POPUP_WINDOW_PADDING();

		ImGuiID id = GetID(str_id);
		if (IsMouseReleased(ContextMenuMouseButton_button) && (IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && WasHoveredWindowHoveredOnMouseClicked(ContextMenuMouseButton_button)) && GetIsMouseSteady())
		{
			if (checkItemHover == IsItemHovered())
				OpenPopupEx(id);
		}
		return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
	}

	void WindowContextMenu(const char* str_id, const std::function<void(void)>& func)
	{
		IM_ASSERT(str_id != nullptr);
		if (InternalBeginContextMenu(str_id, false))
		{
			func();
			EndPopup();
		}
	}

	void ItemContextMenu(const char* str_id, const std::function<void(void)>& func)
	{
		IM_ASSERT(str_id != nullptr);
		if (InternalBeginContextMenu(str_id, true))
		{
			func();
			EndPopup();
		}
	}

	void ExtendedVerticalSeparator(float spacing)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;

		float y1 = window->DC.CursorPos.y;
		float y2 = window->DC.CursorPos.y + window->DC.CurrentLineSize.y;
		const ImRect bb(ImVec2(window->DC.CursorPos.x, y1), ImVec2(window->DC.CursorPos.x + spacing + 1.0f, y2));

		ItemSize(ImVec2(bb.GetWidth(), 0.0f));
		SameLine();

		if (!ItemAdd(bb, 0))
			return;

		float line_height = 1.0f;
		float line_x = bb.Min.x + (spacing * 0.5f);

		window->DrawList->AddLine(ImVec2(line_x, bb.Min.y - line_height), ImVec2(line_x, bb.Max.y + line_height), GetColorU32(ImGuiCol_Separator));
	}

	ExtendedImGuiTextFilter::ExtendedImGuiTextFilter(const char* default_filter) : textFilter(default_filter)
	{
		return;
	}

	bool ExtendedImGuiTextFilter::Draw(const char* label, const char* hint, float width)
	{
		if (width != 0.0f)
			ImGui::PushItemWidth(width);
		bool value_changed = ImGui::InputTextWithHint(label, hint, textFilter.InputBuf, IM_ARRAYSIZE(textFilter.InputBuf));
		if (width != 0.0f)
			ImGui::PopItemWidth();
		if (value_changed)
			Build();
		return value_changed;
	}
}