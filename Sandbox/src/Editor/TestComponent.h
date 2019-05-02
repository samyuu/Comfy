#pragma once
#include "../BaseWindow.h"

class TestComponent : public BaseWindow
{
public:
	TestComponent(Application*);
	~TestComponent();

	virtual const char* GetGuiName() override;
	virtual ImGuiWindowFlags GetWindowFlags() override;
};
