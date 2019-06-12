#pragma once
#include "../BaseWindow.h"

class TestComponent : public BaseWindow
{
public:
	TestComponent(Application*);
	~TestComponent();

	virtual const char* GetGuiName() const override;
	virtual ImGuiWindowFlags GetWindowFlags() const override;
};
