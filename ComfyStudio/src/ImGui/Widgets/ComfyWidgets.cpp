#include "ComfyWidgets.h"
#include "ImGui/Extensions/InternalExtensions.h"

namespace ImGui
{
	constexpr float ColumnWidthFactor = 0.35f;

	struct RAII_ColumnsCount
	{
		RAII_ColumnsCount(int columnsCount, const char* id, bool border)
		{
			ImGuiColumns* previousColumns = GetCurrentWindow()->DC.CurrentColumns;

			if (previousColumns == nullptr)
				PreviousColumn = { 1, nullptr, false };
			else
				PreviousColumn = { previousColumns->Count, nullptr, (previousColumns->Flags & ImGuiColumnsFlags_NoBorder) == 0 };

			Columns(columnsCount, id, border);
		}

		~RAII_ColumnsCount()
		{
			Columns(PreviousColumn.Count, PreviousColumn.ID, PreviousColumn.Border);
		}

		struct PreviousColumnData
		{
			int Count; const char* ID; bool Border;
		} PreviousColumn;
	};

	static void PushDisabledTextColorIfDisabled()
	{
		if (GetCurrentWindow()->DC.ItemFlags & ImGuiItemFlags_Disabled)
			PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));
	}

	static void PopDisabledTextColorIfDisabled()
	{
		if (GetCurrentWindow()->DC.ItemFlags & ImGuiItemFlags_Disabled)
			PopStyleColor();
	}

	bool ComfyDragText(const char* label, float* value, float speed, float min, float max, float width)
	{
		ImGuiWindow* window = GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);

		ImVec2 labelSize = CalcTextSize(label, nullptr, true);
		if (width > 0.0f)
			labelSize.x = 0.0f;

		const ImRect frameBB(window->DC.CursorPos, window->DC.CursorPos + ImVec2(width > 0.0f ? width : GetContentRegionAvailWidth(), labelSize.y + style.FramePadding.y * 2.0f));
		const ImRect totalBB(frameBB.Min, frameBB.Max + ImVec2(labelSize.x > 0.0f ? style.ItemInnerSpacing.x + labelSize.x : 0.0f, 0.0f));

		ItemSize(totalBB, style.FramePadding.y);
		if (!ItemAdd(totalBB, id))
			return false;

		const bool hovered = ItemHoverable(frameBB, id);

		if (hovered && (g.IO.MouseClicked[0] || g.IO.MouseDoubleClicked[0]))
		{
			SetActiveID(id, window);
			SetFocusID(id, window);
			FocusWindow(window);
		}

		const bool valueChanged = DragBehavior(id, ImGuiDataType_Float, value, speed, &min, &max, "", 1.0f, ImGuiDragFlags_None);
		if (valueChanged)
			MarkItemEdited(id);

		const bool isDragging = g.ActiveId == id;
		if (hovered || isDragging)
			SetMouseCursor(ImGuiMouseCursor_ResizeEW);

		// DEBUG:
		// if (isDragging) RenderFrame(frameBB.Min, frameBB.Max, GetColorU32(ImGuiCol_TextSelectedBg, 0.5f), true, style.FrameRounding);

		RenderTextClipped(frameBB.Min, frameBB.Max, label, nullptr, nullptr, ImVec2(0.0f, 0.5f));
		return valueChanged;
	}

	bool ComfyCheckbox(const char* label, bool* value)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushID(value);
		bool valueChanged = Checkbox("##ComfyCheckbox", value);
		PopID();
		NextColumn();

		return valueChanged;
	}

	bool ComfyTextWidget(const char* label, char* buffer, size_t bufferSize, ImGuiInputTextFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: Clickable text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushDisabledTextColorIfDisabled();
		PushItemWidth(GetContentRegionAvailWidth());
		bool valueChanged = InputText("##ComfyInputText", buffer, bufferSize, flags);
		PopItemWidth();
		NextColumn();
		PopDisabledTextColorIfDisabled();

		return valueChanged;
	}

	bool ComfyIntTextWidget(const char* label, int* value, int step, int stepFast, ImGuiInputTextFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: Drag text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushDisabledTextColorIfDisabled();
		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);
		bool valueChanged = InputInt("##ComfyInputInt", value, step, stepFast, flags);
		PopID();
		PopItemWidth();
		PopDisabledTextColorIfDisabled();
		NextColumn();

		return valueChanged;
	}

	bool ComfyInt2TextWidget(const char* label, int value[2], ImGuiInputTextFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: Drag text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);
		bool valueChanged = InputInt2("##ComfyInputInt2", value, flags);
		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyFloatDragWidget(const char* label, float* value, float speed, float minValue, float maxValue, const char* format, bool disabledText)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);

		if (disabledText)
			PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);

		bool valueChanged = DragFloat("##ComfyDragFloat", value, speed, minValue, maxValue, format);

		if (disabledText)
			PopStyleColor();

		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyFloatTextWidget(const char* label, float* value, float step, float stepFast, float min, float max, const char* format, ImGuiInputTextFlags flags, bool disabledText)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		bool valueChanged = ComfyDragText(label, value, step, min, max);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);

		disabledText |= (GetCurrentWindow()->DC.ItemFlags & ImGuiItemFlags_Disabled) != 0;
		if (disabledText)
			PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);

		valueChanged |= InputFloat("##ComfyInputFloat", value, step, stepFast, format, flags);

		if (disabledText)
			PopStyleColor();

		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	static bool ComfyDragTextInputFloat2(const char* label, float value[2], float step, float min, float max, const char* format, ImGuiInputTextFlags flags, const bool disabledText[2] = nullptr)
	{
		// HACK: Doesn't really work with different font sizes
		constexpr float componentLabelWidth = 16.0f;

		const float avaiableWidth = GetContentRegionAvailWidth();
		const float inputFloatWidth = (avaiableWidth * 0.5f) - componentLabelWidth;

		const char* componentLabels[2] = { "  X  ", "  Y  " };
		bool valueChanged = false;

		for (size_t i = 0; i < vec2::length(); i++)
		{
			float* component = &value[i];
			PushID(component);
			
			const bool textDisabled = (disabledText != nullptr && disabledText[i]);
			if (textDisabled)
				PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);

			valueChanged |= ComfyDragText(componentLabels[i], component, step, min, max, componentLabelWidth);
			SameLine(0.0f, 0.0f);

			PushItemWidth(inputFloatWidth);
			valueChanged |= InputFloat("", component, 0.0f, 0.0f, format, flags);
			PopItemWidth();
			SameLine(0.0f, 0.0f);

			if (textDisabled)
				PopStyleColor();

			PopID();
		}

		return valueChanged;
	}

	bool ComfyFloat2TextWidget(const char* label, float value[2], float step, float min, float max, const char* format, ImGuiInputTextFlags flags, const bool disabledText[2])
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		AlignTextToFramePadding();
		Text(label);

		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);

		bool valueChanged = ComfyDragTextInputFloat2("##ComfyDragTextInputFloat2", value, step, min, max, format, flags, disabledText);

		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyColorEdit3(const char* label, float color[3], ImGuiColorEditFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(color);
		bool valueChanged = ColorEdit3("##ComfyColorEdit3", color, flags);
		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfySmallButton(const char* label, const ImVec2& size)
	{
		ImGuiContext& g = *GImGui;
		float backup_padding_y = g.Style.FramePadding.y;
		g.Style.FramePadding.y = 0.0f;
		bool pressed = ButtonEx(label, size, ImGuiButtonFlags_AlignTextBaseLine);
		g.Style.FramePadding.y = backup_padding_y;
		return pressed;
	}

	bool ComfyCenteredButton(const char* label)
	{
		constexpr float centerFactor = 0.5f;

		PushDisabledTextColorIfDisabled();
		SetCursorPosX(GetWindowWidth() * centerFactor * centerFactor);
		bool clicked = Button(label, ImVec2(GetWindowWidth() * centerFactor, 0.0f));
		PopDisabledTextColorIfDisabled();

		return clicked;
	}

	bool ComfyBeginCombo(const char* label, const char* previewValue, ImGuiComboFlags flags)
	{
		// because columns don't function correctly with combo boxes
		Columns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);
		ImVec2 labelPosition = GetCursorScreenPos();
		NextColumn();
		ImVec2 comboPosition = GetCursorScreenPos();
		Columns(1);

		SetCursorScreenPos(labelPosition);
		AlignTextToFramePadding();
		Text(label);

		PushDisabledTextColorIfDisabled();
		SetCursorScreenPos(comboPosition);
		bool open = InternalVariableWidthBeginCombo(label, previewValue, flags, GetContentRegionAvailWidth());
		PopDisabledTextColorIfDisabled();

		return open;
	}

	void ComfyEndCombo()
	{
		EndCombo();
	}

	void ComfyHelpMarker(const char* description)
	{
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

	void ComfySameLineHelpMarker(const char* description)
	{
		SameLine();
		ComfyHelpMarker(description);
	}

	bool ComfyInputFloat(const char* label, float* value, float speed, float min, float max, const char* format, bool disabled)
	{
		if (disabled)
		{
			PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
			{
				float pre_value = *value;
				bool result = DragScalar(label, ImGuiDataType_Float, value, speed, &min, &max, format, 1.0f);
				*value = pre_value;
			}
			PopStyleColor();

			return false;
		}
		else
		{
			return DragScalar(label, ImGuiDataType_Float, value, speed, &min, &max, format, 1.0f);
		}
	}
}