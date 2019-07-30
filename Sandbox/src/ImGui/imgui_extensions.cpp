#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_extensions.h"
#include "Graphics/Texture.h"
#include "FontIcons.h"

namespace ImGui
{
	static ImGuiWindow* activeWindowsOnMouseClicks[5];
	static ImGuiWindow* hoveredWindowsOnMouseClicks[5];

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

	void AddTexture(ImDrawList* drawList, Texture2D* texture, ImVec2 center, ImVec2 scale, const ImVec2& uv0, const ImVec2& uv1)
	{
		float width = texture->GetWidth() * scale.x;
		float height = texture->GetHeight() * scale.y;

		center.x -= width * .5f;
		center.y -= height * .5f;
		ImVec2 bottomRight(center.x + width, center.y + height);

		drawList->AddImage(texture->GetVoidTexture(), center, bottomRight, uv0, uv1);
	}

	void StyleComfy(ImGuiStyle* dst)
	{
		ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
		style->WindowRounding = 0.0f;
		style->FrameBorderSize = 0.0f;

		ImVec4* colors = style->Colors;
		colors[ImGuiCol_Text] = ImVec4(0.88f, 0.88f, 0.88f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.36f, 0.36f, 0.36f, 0.21f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.13f, 0.13f, 0.13f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.37f, 0.37f, 0.37f, 1.00f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.49f, 0.49f, 0.49f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.66f, 0.66f, 0.66f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.27f, 0.37f, 0.13f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.34f, 0.47f, 0.17f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.41f, 0.56f, 0.20f, 0.99f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.27f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.59f, 0.59f, 0.59f, 0.98f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.83f, 0.83f, 0.83f, 1.00f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.50f);
	}

	bool IsItemHoveredDelayed(ImGuiHoveredFlags flags, float threshold)
	{
		return IsItemHovered(flags) && GImGui->HoveredIdTimer > threshold;
	}

	void HelpMarker(const char* description)
	{
		//TextDisabled("(" ICON_FA_QUESTION ")");
		TextDisabled("(?)");
		if (IsItemHovered())
		{
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			TextUnformatted(description);
			PopTextWrapPos();
			EndTooltip();
		}
	}

	void SameLineHelpMarker(const char* description)
	{
		SameLine();
		HelpMarker(description);
	}

	// Same as imgui_widgets.cpp: TreeNodeBehavior(...) but with wider collision detection and a no_arrow paramter
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

	// Same as imgui_widgets.cpp: TreeNodeExV(...) but calls WideTreeNodeBehavior(...) instead of TreeNodeBehavior(...)
	bool WideTreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const char* label_end = g.TempBuffer + ImFormatStringV(g.TempBuffer, IM_ARRAYSIZE(g.TempBuffer), fmt, args);
		return WideTreeNodeBehavior(window->GetID(ptr_id), flags, g.TempBuffer, label_end);
	}

	// Same as imgui_widgets.cpp: TreeNode(...) but calls WideTreeNodeBehavior(...) instead of TreeNodeBehavior(...)
	bool WideTreeNode(const char* label)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), 0, label, NULL);
	}

	// Same as imgui_widgets.cpp: TreeNode(...) but calls WideTreeNodeExV(...) instead of TreeNodeExV(...)
	bool WideTreeNode(const char* str_id, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		bool is_open = WideTreeNodeExV(str_id, 0, fmt, args);
		va_end(args);
		return is_open;
	}

	// Same as imgui_widgets.cpp: TreeNodeEx(...) but calls WideTreeNodeBehavior(...) instead of TreeNodeBehavior(...)
	bool WideTreeNodeEx(const char* label, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), flags, label, NULL);
	}

	// Same as imgui_widgets.cpp: TreeNodeEx(...) but calls WideTreeNodeExV(...) instead of TreeNodeExV(...)
	bool WideTreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		bool is_open = WideTreeNodeExV(ptr_id, flags, fmt, args);
		va_end(args);
		return is_open;
	}

	// Same as WideTreeNode(...) but without the tree arrow / bullet
	bool WideTreeNodeNoArrow(const char* label)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), 0, label, NULL, true);
	}

	// Same as WideTreeNodeEx(...) but without the tree arrow / bullet
	bool WideTreeNodeNoArrow(const char* label, ImGuiTreeNodeFlags flags)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		return WideTreeNodeBehavior(window->GetID(label), flags, label, NULL, true);
	}

	// Same as SmallButton(...) but with a size parameter
	bool SmallButton(const char* label, const ImVec2& size)
	{
		ImGuiContext& g = *GImGui;
		float backup_padding_y = g.Style.FramePadding.y;
		g.Style.FramePadding.y = 0.0f;
		bool pressed = ButtonEx(label, size, ImGuiButtonFlags_AlignTextBaseLine);
		g.Style.FramePadding.y = backup_padding_y;
		return pressed;
	}

	constexpr int ContextMenuMouseButton_button = 1;

	static bool InternalBeginContextMenu(const char* str_id)
	{
		if (!str_id)
			str_id = "window_context";
		ImGuiID id = GImGui->CurrentWindow->GetID(str_id);
		if (IsMouseReleased(ContextMenuMouseButton_button) && IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && WasHoveredWindowHoveredOnMouseClicked(ContextMenuMouseButton_button))
			OpenPopupEx(id);
		return BeginPopupEx(id, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
	}

	void WindowContextMenu(const char* std_id, const std::function<void(void)>& func)
	{
		if (InternalBeginContextMenu(std_id))
		{
			func();
			EndPopup();
		}
	}

	void ItemContextMenu(const char* std_id, const std::function<void(void)>& func)
	{
		if (InternalBeginContextMenu(std_id))
		{
			func();
			EndPopup();
		}
	}

	bool ExtendedInputFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, bool disabled)
	{
		if (disabled)
		{
			PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
			{
				float pre_value = *v;
				bool result = DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, format, 1.0f);
				*v = pre_value;
			}
			PopStyleColor();

			return false;
		}
		else
		{
			return DragScalar(label, ImGuiDataType_Float, v, v_speed, &v_min, &v_max, format, 1.0f);
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
}