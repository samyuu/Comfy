#include "DualShock4.h"
#include <stdio.h>

DualShock4* DualShock4::instance;

DualShock4::DualShock4()
{
}

DualShock4::~DualShock4()
{
	DI_Dispose();
}

bool DualShock4::TryInitializeInstance()
{
	if (InstanceInitialized())
		return true;

	if (!DirectInputInitialized())
		return false;

	DualShock4 *dualShock4 = new DualShock4();

	bool success = dualShock4->Initialize();
	instance = success ? dualShock4 : nullptr;

	if (!success)
		delete dualShock4;

	return success;
}

bool DualShock4::Initialize()
{
	HRESULT result = NULL;

	if (FAILED(result = DI_CreateDevice(GUID_Ds4)))
		return false;

	if (FAILED(result = DI_SetDataFormat(&c_dfDIJoystick2)))
		return false;

	result = DI_Acquire();

	return true;
}

bool DualShock4::PollInput()
{
	lastState = currentState;

	HRESULT result = NULL;
	result = DI_Poll();
	result = DI_GetDeviceState(sizeof(DIJOYSTATE2), &currentState.DI_JoyState);

	if (result != DI_OK)
		return false;

	UpdateInternalDs4State(currentState);

	for (int button = 0; button < DS4_BUTTON_MAX; button++)
		currentState.Buttons[button] = GetButtonState(currentState, (Ds4Button)button);

	return true;
}

void DualShock4::UpdateInternalDs4State(Ds4State &state)
{
	if (state.Dpad.IsDown = state.DI_JoyState.rgdwPOV[0] != -1)
	{
		state.Dpad.Angle = (state.DI_JoyState.rgdwPOV[0] / 100.0f);

		auto direction = GetDirection(state.Dpad.Angle);
		state.Dpad.Stick = { direction.y, -direction.x };
	}
	else
	{
		state.Dpad.Angle = 0;
		state.Dpad.Stick = Joystick();
	}

	state.LeftStick = NormalizeStick(state.DI_JoyState.lX, state.DI_JoyState.lY);
	state.RightStick = NormalizeStick(state.DI_JoyState.lZ, state.DI_JoyState.lRz);

	state.LeftTrigger = { NormalizeTrigger(state.DI_JoyState.lRx) };
	state.RightTrigger = { NormalizeTrigger(state.DI_JoyState.lRy) };
}

bool DualShock4::_IsDown(Ds4Button button)
{
	return currentState.Buttons[button];
}

bool DualShock4::_IsUp(Ds4Button button)
{
	return !_IsDown(button);
}

bool DualShock4::_IsTapped(Ds4Button button)
{
	return _IsDown(button) && _WasUp(button);
}

bool DualShock4::_IsReleased(Ds4Button button)
{
	return _IsUp(button) && _WasDown(button);
}

bool DualShock4::_WasDown(Ds4Button button)
{
	return lastState.Buttons[button];
}

bool DualShock4::_WasUp(Ds4Button button)
{
	return !_WasDown(button);
}

bool DualShock4::MatchesDirection(Joystick joystick, Direction directionEnum, float threshold)
{
	switch (directionEnum)
	{
	case DIR_UP:
		return joystick.YAxis <= -threshold;

	case DIR_RIGHT:
		return joystick.XAxis >= +threshold;

	case DIR_DOWN:
		return joystick.YAxis >= +threshold;

	case DIR_LEFT:
		return joystick.XAxis <= -threshold;

	default:
		return false;
	}
}

bool DualShock4::GetButtonState(Ds4State &state, Ds4Button button)
{
	if (button >= DS4_SQUARE && button <= DS4_TOUCH)
		return state.DI_JoyState.rgbButtons[button];

	if (button >= DS4_DPAD_UP && button <= DS4_DPAD_LEFT)
		return state.Dpad.IsDown ? MatchesDirection(state.Dpad.Stick, (Direction)(button - DS4_DPAD_UP), dpadThreshold) : false;

	if (button >= DS4_L_STICK_UP && button <= DS4_L_STICK_LEFT)
		return MatchesDirection(state.LeftStick, (Direction)(button - DS4_L_STICK_UP), joystickThreshold);

	if (button >= DS4_R_STICK_UP && button <= DS4_R_STICK_LEFT)
		return MatchesDirection(state.RightStick, (Direction)(button - DS4_R_STICK_UP), joystickThreshold);

	return false;
}
