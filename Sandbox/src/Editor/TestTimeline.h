#pragma once
#include "../BaseWindow.h"

class TestTimeline : public BaseWindow
{
public:
	TestTimeline(Application*);
	~TestTimeline();

	virtual const char* GetGuiName() override;
	virtual void DrawGui() override;
	virtual ImGuiWindowFlags GetWindowFlags() override;
};

