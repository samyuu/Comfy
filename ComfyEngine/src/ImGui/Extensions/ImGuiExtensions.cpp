#include "ImGuiExtensions.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Time/Stopwatch.h"
#include "IO/Path.h"

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

	void AddSprite(ImDrawList* drawList, const Comfy::Graphics::SprSet& sprSet, const Comfy::Graphics::Spr& spr, vec2 topLeft, vec2 bottomRight, ImU32 color)
	{
		if (!Comfy::InBounds(spr.TextureIndex, sprSet.TexSet.Textures))
			return;

		const auto& tex = sprSet.TexSet.Textures[spr.TextureIndex];
		const auto uv0 = vec2(spr.TexelRegion.x, 1.0f - spr.TexelRegion.y);
		const auto uv1 = vec2(spr.TexelRegion.x + spr.TexelRegion.z, 1.0f - (spr.TexelRegion.y + spr.TexelRegion.w));

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
					return std::any_of(Comfy::IO::Path::InvalidPathCharacters.begin(), Comfy::IO::Path::InvalidPathCharacters.end(),
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
				const auto roundtripFormat = Comfy::TimeSpan::ParseFormattedTime(data->Buf).FormatTime();
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

	bool InputFormattedTimeSpan(const char* label, Comfy::TimeSpan* value, vec2 size, ImGuiInputTextFlags flags)
	{
		flags |= ImGuiInputTextFlags_NoHorizontalScroll;
		flags |= ImGuiInputTextFlags_AlwaysOverwrite;

		flags |= ImGuiInputTextFlags_CallbackAlways;
		flags |= ImGuiInputTextFlags_CallbackCharFilter;

		auto formattedTime = value->FormatTime();
		const bool result = InputTextEx(label, nullptr, formattedTime.data(), static_cast<i32>(formattedTime.size()), size, flags, InputTextTimeSpanCallback, nullptr);

		if (result)
			*value = Comfy::TimeSpan::ParseFormattedTime(formattedTime.data());

		return result;
	}

	bool WideTreeNodeBehavior(ImGuiID id, ImGuiTreeNodeFlags flags, const char* label, const char* label_end, bool no_arrow = false)
	{
		// HACK: Not sure if this is correct but since it's only used by AetEditor components it's not too important for now
		return ImGui::TreeNodeBehavior(id, flags | /*ImGuiTreeNodeFlags_SpanAvailWidth*/ ImGuiTreeNodeFlags_SpanFullWidth, label, label_end);
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

	void WideTooltip(const std::function<void(void)>& func)
	{
		RAII_POPUP_WINDOW_PADDING();
		BeginTooltip();
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
