#pragma once
#include "ImGui/Extensions/ImguiExtensions.h"

namespace ImGui
{
	bool ComfyDragText(const char* label, float* value, float speed = 1.0f, float min = 0.0f, float max = 0.0f, const float width = 0.0f);

	bool ComfyCheckbox(const char* label, bool* value);
	bool ComfyTextWidget(const char* label, char* buffer, size_t bufferSize, ImGuiInputTextFlags flags = 0);
	
	bool ComfyIntTextWidget(const char* label, int* value, int step, int stepFast, ImGuiInputTextFlags flags = 0);
	bool ComfyInt2TextWidget(const char* label, int value[2], ImGuiInputTextFlags flags = 0);
	
	bool ComfyFloatDragWidget(const char* label, float* value, float speed, float minValue, float maxValue, const char* format = "%.2f", bool disabledText = false);
	bool ComfyFloatTextWidget(const char* label, float* value, float step, float stepFast, float min, float max, const char* format = "%.2f", ImGuiInputTextFlags flags = 0, bool disabledText = false);
	bool ComfyFloat2TextWidget(const char* label, float value[2], float step, float min, float max, const char* format = "%.2f", ImGuiInputTextFlags flags = 0, const bool disabledText[2] = nullptr);
	
	bool ComfyColorEdit3(const char* label, float color[3], ImGuiColorEditFlags flags = 0);

	bool ComfySmallButton(const char* label, const ImVec2& size);
	bool ComfyCenteredButton(const char* label);

	bool ComfyBeginCombo(const char* label, const char* previewValue, ImGuiComboFlags flags = 0);
	void ComfyEndCombo();

	void ComfyHelpMarker(const char* description);
	void ComfySameLineHelpMarker(const char* description);

	bool ComfyInputFloat(const char* label, float* value, float speed, float min, float max, const char* format, bool disabled = false);
}