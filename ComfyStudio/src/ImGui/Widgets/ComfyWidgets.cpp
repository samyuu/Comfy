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

	bool ComfyCheckbox(const char* label, bool* value)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushID(value);
		bool valueChanged = Checkbox("##Checkbox", value);
		PopID();
		NextColumn();

		return valueChanged;
	}

	bool ComfyTextWidget(const char* label, char* buffer, size_t bufferSize, ImGuiInputTextFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		bool valueChanged = InputText("##InputText", buffer, bufferSize, flags);
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyIntWidget(const char* label, int* value, int step, int stepFast, ImGuiInputTextFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: drag text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);
		bool valueChanged = InputInt("##InputInt", value, step, stepFast, flags);
		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyInt2Widget(const char* label, int value[2], ImGuiInputTextFlags flags)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: drag text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);
		bool valueChanged = InputInt2("##InputInt2", value, flags);
		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyFloatWidget(const char* label, float* value, float step, float stepFast, const char* format, ImGuiInputTextFlags flags, bool disabled)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: drag text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);

		if (disabled)
		{
			flags |= ImGuiInputTextFlags_ReadOnly;
			PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
		}

		bool valueChanged = InputFloat("##InputFloat", value, step, stepFast, format, flags);

		if (disabled)
			PopStyleColor();

		PopID();
		PopItemWidth();
		NextColumn();

		return valueChanged;
	}

	bool ComfyFloat2Widget(const char* label, float value[2], const char* format, ImGuiInputTextFlags flags, bool disabled)
	{
		RAII_ColumnsCount raiiColumns(2, nullptr, false);
		SetColumnWidth(0, GetWindowWidth() * ColumnWidthFactor);

		// TODO: drag text
		AlignTextToFramePadding();
		Text(label);
		NextColumn();

		PushItemWidth(GetContentRegionAvailWidth());
		PushID(value);

		if (disabled)
		{
			flags |= ImGuiInputTextFlags_ReadOnly;
			PushStyleColor(ImGuiCol_Text, GImGui->Style.Colors[ImGuiCol_TextDisabled]);
		}

		bool valueChanged = InputFloat2("##InputFloat2", value, format, flags);

		if (disabled)
			PopStyleColor();

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
		bool valueChanged = ColorEdit3("##ColorEdit3", color, flags);
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

		SetCursorPosX(GetWindowWidth() * centerFactor * centerFactor);
		bool clicked = Button(label, ImVec2(GetWindowWidth() * centerFactor, 0.0f));
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

		bool disabled = GetCurrentWindow()->DC.ItemFlags & ImGuiItemFlags_Disabled;
		if (disabled)
			PushStyleColor(ImGuiCol_Text, GetStyleColorVec4(ImGuiCol_TextDisabled));

		SetCursorScreenPos(comboPosition);
		bool open = InternalVariableWidthBeginCombo(label, previewValue, flags, GetContentRegionAvailWidth());
		
		if (disabled)
			PopStyleColor();
		
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