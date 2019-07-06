#pragma once
#include "ImGui/imgui.h"

class Application;

class BaseWindow
{
public:
	BaseWindow(Application* parent);
	virtual ~BaseWindow();

	virtual const char* GetGuiName() const = 0;
	virtual void DrawGui() {};
	virtual ImGuiWindowFlags GetWindowFlags() const;

	inline bool* GetIsGuiOpenPtr() { return &isGuiOpen; };
	inline Application* GetParent() const { return parentApplication; };

	static inline ImGuiWindowFlags GetNoWindowFlags()
	{
		return ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoBringToFrontOnFocus;
	}

private:
	bool isGuiOpen = true;
	Application* parentApplication = nullptr;
};

