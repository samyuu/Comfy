#pragma once
#include "ImGui/imgui.h"

class Application;

class BaseWindow
{
public:
	BaseWindow(Application* parent);
	~BaseWindow();

	virtual const char* GetGuiName() const = 0;
	virtual void DrawGui() {};
	virtual ImGuiWindowFlags GetWindowFlags() const;

	inline bool* GetIsGuiOpenPtr() { return &isGuiOpen; };
	inline Application* GetParent() const { return parentApplication; };

private:
	bool isGuiOpen = true;
	Application* parentApplication = nullptr;
};

