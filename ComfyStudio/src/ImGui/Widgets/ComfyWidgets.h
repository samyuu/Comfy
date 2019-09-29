#pragma once
#include "ImGui/Extensions/ImguiExtensions.h"

namespace ImGui
{
	bool ComfyCheckbox(const char* label, bool* value);
	bool ComfyTextWidget(const char* label, char* buffer, size_t bufferSize, ImGuiInputTextFlags flags = 0);
	bool ComfyIntWidget(const char* label, int* value, int step, int stepFast, ImGuiInputTextFlags flags = 0);
	bool ComfyInt2Widget(const char* label, int value[2], ImGuiInputTextFlags flags = 0);
	
	bool ComfyFloatWidget(const char* label, float* value, float step, float stepFast, const char* format = "%.2f", ImGuiInputTextFlags flags = 0, bool disabledText = false);
	bool ComfyFloat2Widget(const char* label, float value[2], const char* format = "%.2f", ImGuiInputTextFlags flags = 0, bool disabledText = false);
	
	bool ComfyColorEdit3(const char* label, float color[3], ImGuiColorEditFlags flags = 0);

	bool ComfySmallButton(const char* label, const ImVec2& size);
	bool ComfyCenteredButton(const char* label);

	bool ComfyBeginCombo(const char* label, const char* previewValue, ImGuiComboFlags flags = 0);
	void ComfyEndCombo();

	void ComfyHelpMarker(const char* description);
	void ComfySameLineHelpMarker(const char* description);

	bool ComfyInputFloat(const char* label, float* value, float speed, float min, float max, const char* format, bool disabled = false);
}