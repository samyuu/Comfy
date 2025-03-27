#include "ImGuiExtensions.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Time/Stopwatch.h"
#include "IO/Path.h"

using namespace Comfy;
using namespace Comfy::Graphics;

namespace ImGui
{
	static ImGuiWindow* activeWindowsOnMouseClicks[5];
	static ImGuiWindow* hoveredWindowsOnMouseClicks[5];

	struct RAII_PopupWindowPadding
	{
		RAII_PopupWindowPadding() { PushStyleVar(ImGuiStyleVar_WindowPadding, PopupWindowPadding); }
		~RAII_PopupWindowPadding() { PopStyleVar(); }
	};

#define RAII_POPUP_WINDOW_PADDING() RAII_PopupWindowPadding COMFY_UNIQUENAME(__RAII_POPUP_WINDOW_PADDING)

	ImRect FitFixedAspectRatio(ImRect sourceRegion, float targetAspectRatio)
	{
		constexpr f32 roundingAdd = 0.0f; // 0.5f;

		const auto sourceSize = vec2(sourceRegion.GetSize());
		const auto sourceAspectRatio = sourceSize.x / sourceSize.y;

		if (sourceAspectRatio <= targetAspectRatio) // NOTE: Taller than wide, bars on top / bottom
		{
			const auto presentHeight = glm::round((sourceSize.x / targetAspectRatio) + roundingAdd);
			const auto barHeight = glm::round((sourceSize.y - presentHeight) / 2.0f);

			sourceRegion.Min.y += barHeight;
			sourceRegion.Max.y += barHeight;
			sourceRegion.Max.y = sourceRegion.Min.y + presentHeight;
		}
		else // NOTE: Wider than tall, bars on left / right
		{
			const auto presentWidth = static_cast<int>((sourceSize.y * targetAspectRatio) + roundingAdd);
			const auto barWidth = static_cast<int>((sourceSize.x - presentWidth) / 2.0f);

			sourceRegion.Min.x += barWidth;
			sourceRegion.Max.x += barWidth;
			sourceRegion.Max.x = sourceRegion.Min.x + presentWidth;
		}

		return sourceRegion;
	}

	ImRect FitFixedAspectRatioImage(ImRect sourceRegion, vec2 imageDimensions)
	{
		return FitFixedAspectRatio(sourceRegion, (imageDimensions.x / imageDimensions.y));
	}


	f32 AnimateFadeInOutOpacity(f32& inOutCurrentOpacity, bool isFadingIn, f32 fadeInDurationSec, f32 fadeOutDurationSec, f32 minOpacity, f32 maxOpacity)
	{
		const f32 deltaScaledOpacityRange = (maxOpacity - minOpacity) * GetIO().DeltaTime;

		return inOutCurrentOpacity = isFadingIn ?
			Min(inOutCurrentOpacity + (deltaScaledOpacityRange / fadeInDurationSec), maxOpacity) :
			Max(inOutCurrentOpacity - (deltaScaledOpacityRange / fadeOutDurationSec), minOpacity);
	}

	f32 AnimateFadeInOutOpacity(f32& inOutCurrentOpacity, bool isFadingIn, TimeSpan fadeInDuration, TimeSpan fadeOutDuration, f32 minOpacity, f32 maxOpacity)
	{
		return AnimateFadeInOutOpacity(inOutCurrentOpacity, isFadingIn, static_cast<f32>(fadeInDuration.TotalSeconds()), static_cast<f32>(fadeOutDuration.TotalSeconds()), minOpacity, maxOpacity);
	}

	f32 AnimateFadeInOutOpacity(f32& inOutCurrentOpacity, bool isFadingIn, TimeSpan fadeInOutDuration, f32 minOpacity, f32 maxOpacity)
	{
		return AnimateFadeInOutOpacity(inOutCurrentOpacity, isFadingIn, static_cast<f32>(fadeInOutDuration.TotalSeconds()), static_cast<f32>(fadeInOutDuration.TotalSeconds()), minOpacity, maxOpacity);
	}

	void TooltipFadeInOutHelper::ResetFade(const void* newID)
	{
		CurrentID = newID;
		CurrentOpacity = 0.0f;
		CurrentElapsedHoverTime = TimeSpan::Zero();
	}

	f32 TooltipFadeInOutHelper::UpdateFadeAndGetOpacity(const void* newID, bool isItemHovered)
	{
		if (CurrentID == newID)
			CurrentElapsedHoverTime = isItemHovered ? (CurrentElapsedHoverTime + TimeSpan::FromSeconds(GetIO().DeltaTime)) : TimeSpan::Zero();
		else if (isItemHovered)
			ResetFade(newID);

		return (CurrentID == newID) ? AnimateFadeInOutOpacity(CurrentOpacity, (CurrentElapsedHoverTime > FadeInDelay), FadeInDuration, FadeOutDuration, MinOpacity, MaxOpacity) : 0.0f;
	}

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

	void PushItemDisabledAndTextColorIf(bool condition)
	{
		if (condition)
			PushItemDisabledAndTextColor();
	}

	void PopItemDisabledAndTextColorIf(bool condition)
	{
		if (condition)
			PopItemDisabledAndTextColor();
	}

	void PushItemDisabledAndTextColor()
	{
		PushItemFlag(ImGuiItemFlags_Disabled, true);
		PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
	}

	void PopItemDisabledAndTextColor()
	{
		PopStyleColor();
		PopItemFlag();
	}

	ImVec4 FindOriginalColorVec4BeforeColorStackEdits(ImGuiCol colorEnum)
	{
		for (const auto& colorMod : GetCurrentContext()->ColorStack)
		{
			if (colorMod.Col == colorEnum)
				return colorMod.BackupValue;
		}

		return GetStyleColorVec4(colorEnum);
	}

	ImU32 FindOriginalColorU32BeforeColorStackEdits(ImGuiCol colorEnum)
	{
		return ColorConvertFloat4ToU32(FindOriginalColorVec4BeforeColorStackEdits(colorEnum));
	}

	void AddTextWithShadow(ImDrawList* drawList, vec2 position, std::string_view text, u32 color, u32 shadowColor, vec2 shadowOffset)
	{
		drawList->AddText(position + shadowOffset, shadowColor, StringViewStart(text), StringViewEnd(text));
		drawList->AddText(position, color, StringViewStart(text), StringViewEnd(text));
	}

	void AddTexture(ImDrawList* drawList, const Tex* tex, vec2 center, vec2 scale, vec2 uv0, vec2 uv1)
	{
		vec2 size = vec2(tex->GetSize()) * scale;

		center -= size * 0.5f;
		vec2 bottomRight = center + size;

		drawList->AddImage(*tex, center, bottomRight, uv0, uv1);
	}

	void AddSprite(ImDrawList* drawList, const Tex* tex, vec2 position, const vec4& sourceRegion, ImU32 color)
	{
		const vec2 textureSize = tex->GetSize();

		vec2 uv0, uv1;
		uv0.x = (sourceRegion.x / textureSize.x);
		uv1.x = uv0.x + (sourceRegion.z / textureSize.x);

		uv0.y = 1.0f - (sourceRegion.y / textureSize.y);
		uv1.y = uv0.y + (sourceRegion.w / textureSize.y);

		drawList->AddImage(*tex, position, position + vec2(sourceRegion.z, sourceRegion.w), uv0, uv1, color);
	}

	void AddSprite(ImDrawList* drawList, const Graphics::SprSet& sprSet, const Graphics::Spr& spr, vec2 topLeft, vec2 bottomRight, ImU32 color)
	{
		if (!InBounds(spr.TextureIndex, sprSet.TexSet.Textures))
			return;

		const auto& tex = sprSet.TexSet.Textures[spr.TextureIndex];
		const vec2 uv0 = vec2(spr.TexelRegion.x, 1.0f - spr.TexelRegion.y);
		const vec2 uv1 = vec2(spr.TexelRegion.z, 1.0f - spr.TexelRegion.w);

		drawList->AddImage(*tex, topLeft, bottomRight, uv0, uv1, color);
	}

	void AddLine(ImDrawList* drawList, vec2 start, vec2 end, ImU32 color, float thickness)
	{
		drawList->AddLine(glm::round(start), glm::round(end), color, thickness);
	}

	void AddQuadFilled(ImDrawList* drawList, vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, ImU32 color)
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

	namespace
	{
		struct InputTextStdStringCallbackUserData
		{
			std::string* Str;
			ImGuiInputTextCallback ChainCallback;
			void* ChainCallbackUserData;
		};

		int InputTextStdStringCallback(ImGuiInputTextCallbackData* data)
		{
			InputTextStdStringCallbackUserData* user_data = (InputTextStdStringCallbackUserData*)data->UserData;
			if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
			{
				std::string* str = user_data->Str;
				IM_ASSERT(data->Buf == str->c_str());
				str->resize(data->BufTextLen);
				data->Buf = (char*)str->c_str();
			}
			else if (user_data->ChainCallback)
			{
				data->UserData = user_data->ChainCallbackUserData;
				return user_data->ChainCallback(data);
			}
			return 0;
		}

		int ValidPathCharTextCallbackFilter(ImGuiInputTextCallbackData* data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
			{
				auto isInvalidPathChar = [](char charToCheck)
				{
					return std::any_of(IO::Path::InvalidPathCharacters.begin(), IO::Path::InvalidPathCharacters.end(),
						[charToCheck](char invalidChar) { return (charToCheck == invalidChar); });
				};

				if (data->EventChar <= std::numeric_limits<u8>::max() && isInvalidPathChar(static_cast<char>(data->EventChar)))
					return 1;
			}

			return 0;
		}
	}

	bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextStdStringCallbackUserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextStdStringCallback, &cb_user_data);
	}

	bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextStdStringCallbackUserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextStdStringCallback, &cb_user_data);
	}

	bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;

		InputTextStdStringCallbackUserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = callback;
		cb_user_data.ChainCallbackUserData = user_data;
		return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextStdStringCallback, &cb_user_data);
	}

	bool PathInputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags)
	{
		IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
		flags |= ImGuiInputTextFlags_CallbackResize;
		flags |= ImGuiInputTextFlags_CallbackCharFilter;

		InputTextStdStringCallbackUserData cb_user_data;
		cb_user_data.Str = str;
		cb_user_data.ChainCallback = ValidPathCharTextCallbackFilter;
		cb_user_data.ChainCallbackUserData = nullptr;
		return InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextStdStringCallback, &cb_user_data);
	}

	namespace
	{
		int InputTextTimeSpanCallback(ImGuiInputTextCallbackData* data)
		{
			if (data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
			{
				// NOTE: To immediately restore any invalid input such as when deleting the ':' or '.'
				const auto roundtripFormat = TimeSpan::ParseFormattedTime(data->Buf).FormatTime();
				const size_t roundTripLength = strlen(roundtripFormat.data());

				assert(roundTripLength <= data->BufSize);
				std::memcpy(data->Buf, roundtripFormat.data(), roundTripLength + 1);
				data->BufTextLen = static_cast<i32>(roundTripLength);
				data->BufDirty = true;
			}
			else if (data->EventFlag == ImGuiInputTextFlags_CallbackCharFilter)
			{
				if (data->EventChar >= '0' && data->EventChar <= '9')
					return 0;
				if (data->EventChar == '+' || data->EventChar == '-' || data->EventChar == ':' || data->EventChar == '.')
					return 0;

				data->EventChar = '\0';
			}

			return 0;
		}
	}

	bool InputFormattedTimeSpan(const char* label, TimeSpan* value, vec2 size, ImGuiInputTextFlags flags)
	{
		flags |= ImGuiInputTextFlags_NoHorizontalScroll;
		flags |= ImGuiInputTextFlags_AlwaysOverwrite;

		flags |= ImGuiInputTextFlags_CallbackAlways;
		flags |= ImGuiInputTextFlags_CallbackCharFilter;

		auto formattedTime = value->FormatTime();
		const bool result = InputTextEx(label, nullptr, formattedTime.data(), static_cast<i32>(formattedTime.size()), size, flags, InputTextTimeSpanCallback, nullptr);

		if (result)
			*value = TimeSpan::ParseFormattedTime(formattedTime.data());

		return result;
	}

	bool WideTreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, bool no_arrow = false)
	{
#if 1 // HACK: I don't even know anymore...
		flags |= (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth);
#endif

		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const bool display_frame = (flags & ImGuiTreeNodeFlags_Framed) != 0;
		const ImVec2 padding = (display_frame || (flags & ImGuiTreeNodeFlags_FramePadding)) ? style.FramePadding : ImVec2(style.FramePadding.x, ImMin(window->DC.CurrLineTextBaseOffset, style.FramePadding.y));

		if (!label_end)
			label_end = FindRenderedTextEnd(label);
		const ImVec2 label_size = CalcTextSize(label, label_end, false);

		// We vertically grow up to current line height up the typical widget height.
		const float frame_height = ImMax(ImMin(window->DC.CurrLineSize.y, g.FontSize + style.FramePadding.y * 2), label_size.y + padding.y * 2);
		ImRect frame_bb;
		frame_bb.Min.x = (flags & ImGuiTreeNodeFlags_SpanFullWidth) ? window->WorkRect.Min.x : window->DC.CursorPos.x;
		frame_bb.Min.y = window->DC.CursorPos.y;
		frame_bb.Max.x = window->WorkRect.Max.x;
		frame_bb.Max.y = window->DC.CursorPos.y + frame_height;

		if (display_frame)
		{
			// Framed header expand a little outside the default padding, to the edge of InnerClipRect
			// (FIXME: May remove this at some point and make InnerClipRect align with WindowPadding.x instead of WindowPadding.x*0.5f)
			frame_bb.Min.x -= IM_FLOOR(window->WindowPadding.x * 0.5f - 1.0f);
			frame_bb.Max.x += IM_FLOOR(window->WindowPadding.x * 0.5f);
		}

		const float text_offset_x = g.FontSize + (display_frame ? padding.x * 3 : padding.x * 2);           // Collapser arrow width + Spacing
		const float text_offset_y = ImMax(padding.y, window->DC.CurrLineTextBaseOffset);                    // Latch before ItemSize changes it

#if 1 // HACK: This seems to do the trick I guess.. more or less. The new Dear ImGui update seems to have fucked up column padding *everywhere*. I fucking hate this shit :PepoKms:
		{
			frame_bb.Min.x -= style.ItemSpacing.x;
			frame_bb.Min.y -= style.ItemSpacing.y;
			frame_bb.Max.x += style.ItemSpacing.x;
			frame_bb.Max.y += style.ItemSpacing.y;
			frame_bb.Max.y -= 1.0f;
		}
#endif

		const float text_width = g.FontSize + (label_size.x > 0.0f ? label_size.x + padding.x * 2 : 0.0f);  // Include collapser
		ImVec2 text_pos(window->DC.CursorPos.x + text_offset_x, window->DC.CursorPos.y + text_offset_y);
		ItemSize(ImVec2(text_width, frame_height), padding.y);

		// For regular tree nodes, we arbitrary allow to click past 2 worth of ItemSpacing
		ImRect interact_bb = frame_bb;
		if (!display_frame && (flags & (ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth)) == 0)
			interact_bb.Max.x = frame_bb.Min.x + text_width + style.ItemSpacing.x * 2.0f;

		// Store a flag for the current depth to tell if we will allow closing this node when navigating one of its child.
		// For this purpose we essentially compare if g.NavIdIsAlive went from 0 to 1 between TreeNode() and TreePop().
		// This is currently only support 32 level deep and we are fine with (1 << Depth) overflowing into a zero.
		const bool is_leaf = (flags & ImGuiTreeNodeFlags_Leaf) != 0;
		bool is_open = TreeNodeBehaviorIsOpen(id, flags);
		if (is_open && !g.NavIdIsAlive && (flags & ImGuiTreeNodeFlags_NavLeftJumpsBackHere) && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			window->DC.TreeJumpToParentOnPopMask |= (1 << window->DC.TreeDepth);

		bool item_add = ItemAdd(interact_bb, id);
		window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_HasDisplayRect;
		window->DC.LastItemDisplayRect = frame_bb;

		if (!item_add)
		{
			if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
				TreePushOverrideID(id);
			IMGUI_TEST_ENGINE_ITEM_INFO(window->DC.LastItemId, label, window->DC.LastItemStatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
			return is_open;
		}

		ImGuiButtonFlags button_flags = ImGuiTreeNodeFlags_None;
		if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
			button_flags |= ImGuiButtonFlags_AllowItemOverlap;
		if (!is_leaf)
			button_flags |= ImGuiButtonFlags_PressedOnDragDropHold;

		// We allow clicking on the arrow section with keyboard modifiers held, in order to easily
		// allow browsing a tree while preserving selection with code implementing multi-selection patterns.
		// When clicking on the rest of the tree node we always disallow keyboard modifiers.
		const float arrow_hit_x1 = (text_pos.x - text_offset_x) - style.TouchExtraPadding.x;
		const float arrow_hit_x2 = (text_pos.x - text_offset_x) + (g.FontSize + padding.x * 2.0f) + style.TouchExtraPadding.x;
		const bool is_mouse_x_over_arrow = (g.IO.MousePos.x >= arrow_hit_x1 && g.IO.MousePos.x < arrow_hit_x2);
		if (window != g.HoveredWindow || !is_mouse_x_over_arrow)
			button_flags |= ImGuiButtonFlags_NoKeyModifiers;

		// Open behaviors can be altered with the _OpenOnArrow and _OnOnDoubleClick flags.
		// Some alteration have subtle effects (e.g. toggle on MouseUp vs MouseDown events) due to requirements for multi-selection and drag and drop support.
		// - Single-click on label = Toggle on MouseUp (default, when _OpenOnArrow=0)
		// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=0)
		// - Single-click on arrow = Toggle on MouseDown (when _OpenOnArrow=1)
		// - Double-click on label = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1)
		// - Double-click on arrow = Toggle on MouseDoubleClick (when _OpenOnDoubleClick=1 and _OpenOnArrow=0)
		// It is rather standard that arrow click react on Down rather than Up.
		// We set ImGuiButtonFlags_PressedOnClickRelease on OpenOnDoubleClick because we want the item to be active on the initial MouseDown in order for drag and drop to work.
		if (is_mouse_x_over_arrow)
			button_flags |= ImGuiButtonFlags_PressedOnClick;
		else if (flags & ImGuiTreeNodeFlags_OpenOnDoubleClick)
			button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick;
		else
			button_flags |= ImGuiButtonFlags_PressedOnClickRelease;

		bool selected = (flags & ImGuiTreeNodeFlags_Selected) != 0;
		const bool was_selected = selected;

		bool hovered, held;
		bool pressed = ButtonBehavior(interact_bb, id, &hovered, &held, button_flags);
		bool toggled = false;
		if (!is_leaf)
		{
			if (pressed && g.DragDropHoldJustPressedId != id)
			{
				if ((flags & (ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick)) == 0 || (g.NavActivateId == id))
					toggled = true;
				if (flags & ImGuiTreeNodeFlags_OpenOnArrow)
					toggled |= is_mouse_x_over_arrow && !g.NavDisableMouseHover; // Lightweight equivalent of IsMouseHoveringRect() since ButtonBehavior() already did the job
				if ((flags & ImGuiTreeNodeFlags_OpenOnDoubleClick) && g.IO.MouseDoubleClicked[0])
					toggled = true;
			}
			else if (pressed && g.DragDropHoldJustPressedId == id)
			{
				IM_ASSERT(button_flags & ImGuiButtonFlags_PressedOnDragDropHold);
				if (!is_open) // When using Drag and Drop "hold to open" we keep the node highlighted after opening, but never close it again.
					toggled = true;
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
				window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledOpen;
			}
		}
		if (flags & ImGuiTreeNodeFlags_AllowItemOverlap)
			SetItemAllowOverlap();

		// In this branch, TreeNodeBehavior() cannot toggle the selection so this will never trigger.
		if (selected != was_selected) //-V547
			window->DC.LastItemStatusFlags |= ImGuiItemStatusFlags_ToggledSelection;

		// Render
		const ImU32 text_col = GetColorU32(ImGuiCol_Text);
		ImGuiNavHighlightFlags nav_highlight_flags = ImGuiNavHighlightFlags_TypeThin;
		if (display_frame)
		{
			// Framed type
			const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
			RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, true, style.FrameRounding);
			RenderNavHighlight(frame_bb, id, nav_highlight_flags);
			if (flags & ImGuiTreeNodeFlags_Bullet)
				RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.60f, text_pos.y + g.FontSize * 0.5f), text_col);
			else if (!is_leaf)
				RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 1.0f);
			else // Leaf without bullet, left-adjusted text
				text_pos.x -= text_offset_x;
			if (flags & ImGuiTreeNodeFlags_ClipLabelForTrailingButton)
				frame_bb.Max.x -= g.FontSize + style.FramePadding.x;

			if (g.LogEnabled)
				LogSetNextTextDecoration("###", "###");
			RenderTextClipped(text_pos, frame_bb.Max, label, label_end, &label_size);
		}
		else
		{
			// Unframed typed for tree nodes
			if (hovered || selected)
			{
				const ImU32 bg_col = GetColorU32((held && hovered) ? ImGuiCol_HeaderActive : hovered ? ImGuiCol_HeaderHovered : ImGuiCol_Header);
				RenderFrame(frame_bb.Min, frame_bb.Max, bg_col, false);
				RenderNavHighlight(frame_bb, id, nav_highlight_flags);
			}
			if (flags & ImGuiTreeNodeFlags_Bullet)
				RenderBullet(window->DrawList, ImVec2(text_pos.x - text_offset_x * 0.5f, text_pos.y + g.FontSize * 0.5f), text_col);
			else if (!is_leaf)
				RenderArrow(window->DrawList, ImVec2(text_pos.x - text_offset_x + padding.x, text_pos.y + g.FontSize * 0.15f), text_col, is_open ? ImGuiDir_Down : ImGuiDir_Right, 0.70f);
			if (g.LogEnabled)
				LogSetNextTextDecoration(">", NULL);
			RenderText(text_pos, label, label_end, false);
		}

		if (is_open && !(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen))
			TreePushOverrideID(id);
		IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags | (is_leaf ? 0 : ImGuiItemStatusFlags_Openable) | (is_open ? ImGuiItemStatusFlags_Opened : 0));
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

	bool WideBeginPopup(const char* label, ImGuiWindowFlags flags)
	{
		RAII_POPUP_WINDOW_PADDING();
		return BeginPopup(label, flags);
	}

	bool WideBeginPopupModal(const char* name, bool* p_open, ImGuiWindowFlags flags)
	{
		PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(/*GImGui->Style.FramePadding.x*/8.0f, 5.0f));
		const bool result = BeginPopupModal(name, p_open, flags);
		PopStyleVar();
		return result;
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

	bool MenuItemDontClosePopup(const char* label, const char* shortcut, bool selected, bool enabled)
	{
		PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
		const bool result = MenuItem(label, shortcut, selected, enabled);
		PopItemFlag();
		return result;
	}

	bool MenuItemDontClosePopup(const char* label, const char* shortcut, bool* selected, bool enabled)
	{
		// BUG: If the menu item is used to control the open state of a window and the window is opened on the next frame 
		//		then the popup is closed anyway (due to the focus loss..?)
		PushItemFlag(ImGuiItemFlags_SelectableDontClosePopup, true);
		const bool result = MenuItem(label, shortcut, selected, enabled);
		PopItemFlag();
		return result;
	}

	void SetWideItemTooltip(const char* fmt, ...)
	{
		if (IsItemHoveredDelayed())
		{
			RAII_POPUP_WINDOW_PADDING();

			va_list args;
			va_start(args, fmt);
			SetTooltipV(fmt, args);
			va_end(args);
		}
	}

	void WideSetTooltip(const char* fmt, ...)
	{
		RAII_POPUP_WINDOW_PADDING();

		va_list args;
		va_start(args, fmt);
		SetTooltipV(fmt, args);
		va_end(args);
	}

	namespace
	{
		void ModifiedBeginTooltipExWithExplicitPositionAndPivot(vec2 position, vec2 pivot)
		{
			ImGuiContext& g = *GImGui;

			// if (g.DragDropWithinSource || g.DragDropWithinTarget)
			{
				ImVec2 tooltip_pos = position; // g.IO.MousePos + ImVec2(16 * g.Style.MouseCursorScale, 8 * g.Style.MouseCursorScale);
				SetNextWindowPos(tooltip_pos, ImGuiCond_None, pivot);
				SetNextWindowBgAlpha(g.Style.Colors[ImGuiCol_PopupBg].w * 0.60f);
			}

			char window_name[16];
			ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##Tooltip_%02d", g.TooltipOverrideCount);

			if (ImGuiWindow* window = FindWindowByName(window_name); window != nullptr && window->Active)
			{
				window->Hidden = true;
				window->HiddenFramesCanSkipItems = 1;
				ImFormatString(window_name, IM_ARRAYSIZE(window_name), "##Tooltip_%02d", ++g.TooltipOverrideCount);
			}

			ImGuiWindowFlags flags = ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDocking;
			Begin(window_name, nullptr, flags);
		}
	}

	void WideSetTooltip(vec2 position, vec2 pivot, const char* fmt, ...)
	{
		RAII_POPUP_WINDOW_PADDING();

		va_list args;
		va_start(args, fmt);

		ModifiedBeginTooltipExWithExplicitPositionAndPivot(position, pivot);
		TextV(fmt, args);
		EndTooltip();

		va_end(args);
	}

	void WideTooltip(const std::function<void(void)>& func)
	{
		RAII_POPUP_WINDOW_PADDING();
		BeginTooltip();
		func();
		EndTooltip();
	}

	void WideTooltip(vec2 position, vec2 pivot, const std::function<void(void)>& func)
	{
		RAII_POPUP_WINDOW_PADDING();

		ModifiedBeginTooltipExWithExplicitPositionAndPivot(position, pivot);
		func();
		EndTooltip();
	}

	void HelpMarker(std::string_view description)
	{
		TextDisabled("(?)");
		if (IsItemHovered())
		{
			BeginTooltip();
			PushTextWrapPos(GetFontSize() * 35.0f);
			TextUnformatted(StringViewStart(description), StringViewEnd(description));
			PopTextWrapPos();
			EndTooltip();
		}
	}

	void SameLineHelpMarker(std::string_view description)
	{
		SameLine();
		HelpMarker(description);
	}

	void SameLineHelpMarker(float localPosX, float spacingWidth, std::string_view description)
	{
		SameLine(localPosX, spacingWidth);
		HelpMarker(description);
	}

	void SameLineHelpMarkerRightAlign(std::string_view description)
	{
		SameLine(GetWindowWidth() - (GetFontSize() + 2.0f), 0.0f);
		HelpMarker(description);
	}

	constexpr int ContextMenuMouseButton_button = 1;

	bool IsMouseSteady()
	{
		constexpr float threshold = 2.0f;

		vec2 mouseDragDelta = GetMouseDragDelta(1);
		return fabs(mouseDragDelta.x) < threshold && fabs(mouseDragDelta.y) < threshold;
	}

	static bool InternalBeginContextMenu(const char* str_id, bool checkItemHover)
	{
		RAII_POPUP_WINDOW_PADDING();

		ImGuiID id = GetID(str_id);
		if (IsMouseReleased(ContextMenuMouseButton_button) && (IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup) && WasHoveredWindowHoveredOnMouseClicked(ContextMenuMouseButton_button)) && IsMouseSteady())
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

