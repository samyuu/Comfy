#pragma once
#include "BaseWindow.h"
#include "Input/DirectInput/DualShock4.h"

class InputTestWindow : public BaseWindow
{
public:
	InputTestWindow(Application*);
	~InputTestWindow();

	virtual void DrawGui() override;
	virtual const char* GetGuiName() const override;
	virtual ImGuiWindowFlags GetWindowFlags() const override;

private:

	const char* ds4ButtonNames[DS4_BUTTON_MAX] =
	{
		"DS4_SQUARE",
		"DS4_CROSS",
		"DS4_CIRCLE",
		"DS4_TRIANGLE",
		"DS4_L1",
		"DS4_R1",
		"DS4_L_TRIGGER",
		"DS4_R_TRIGGER",
		"DS4_SHARE",
		"DS4_OPTIONS",
		"DS4_L3",
		"DS4_R3",
		"DS4_PS",
		"DS4_TOUCH",
		"DS4_DPAD_UP",
		"DS4_DPAD_RIGHT",
		"DS4_DPAD_DOWN",
		"DS4_DPAD_LEFT",
		"DS4_L_STICK_UP",
		"DS4_L_STICK_RIGHT",
		"DS4_L_STICK_DOWN",
		"DS4_L_STICK_LEFT",
		"DS4_R_STICK_UP",
		"DS4_R_STICK_RIGHT",
		"DS4_R_STICK_DOWN",
		"DS4_R_STICK_LEFT",
	};

	void RefreshDevices();
};