#pragma once
#include "ImGui/imgui.h"

class Application;

class BaseWindow
{
public:
	BaseWindow(Application* parent);
	~BaseWindow();

	virtual const char* GetGuiName() = 0;
	virtual void DrawGui() {};
	virtual ImGuiWindowFlags GetWindowFlags();

	inline bool* GetIsGuiOpenPtr() { return &isGuiOpen; };
	inline Application* GetParent() { return parentApplication; };

private:
	bool isGuiOpen = true;
	Application* parentApplication = nullptr;
};

