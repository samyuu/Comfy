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
	if (GetInstanceInitialized())
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

	const size_t guidCount = sizeof(GUID_Ds4) / sizeof(GUID);
	for (size_t i = 0; i < guidCount; i++)
	{
		result = DI_CreateDevice(GUID_Ds4[i]);

		if (!FAILED(result))
			break;
		else if (i == guidCount - 1)
			return false;
	}

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

	for (int32_t button = 0; button < static_cast<int32_t>(Ds4Button::Count); button++)
		currentState.Buttons[button] = GetButtonState(currentState, static_cast<Ds4Button>(button));

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
	return currentState.Buttons[static_cast<int32_t>(button)];
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
	return lastState.Buttons[static_cast<int32_t>(button)];
}

bool DualShock4::_WasUp(Ds4Button button)
{
	return !_WasDown(button);
}

bool DualShock4::MatchesDirection(Joystick joystick, Direction directionEnum, float threshold)
{
	switch (directionEnum)
	{
	case Direction::Up:
		return joystick.YAxis <= -threshold;

	case Direction::Right:
		return joystick.XAxis >= +threshold;

	case Direction::Down:
		return joystick.YAxis >= +threshold;

	case Direction::Left:
		return joystick.XAxis <= -threshold;

	default:
		return false;
	}
}

bool DualShock4::GetButtonState(Ds4State &state, Ds4Button button)
{
	if (button >= Ds4Button::Square && button <= Ds4Button::Touch)
		return state.DI_JoyState.rgbButtons[static_cast<int32_t>(button)];

	if (button >= Ds4Button::DPad_Up && button <= Ds4Button::DPad_Left)
		return state.Dpad.IsDown ? MatchesDirection(state.Dpad.Stick, static_cast<Direction>(static_cast<int32_t>(button) - static_cast<int32_t>(Ds4Button::DPad_Up)), dpadThreshold) : false;

	if (button >= Ds4Button::L_Stick_Up && button <= Ds4Button::L_Stick_Left)
		return MatchesDirection(state.LeftStick, (Direction)(static_cast<int32_t>(button) - static_cast<int32_t>(Ds4Button::L_Stick_Up)), joystickThreshold);

	if (button >= Ds4Button::R_Stick_Up && button <= Ds4Button::R_Stick_Left)
		return MatchesDirection(state.RightStick, (Direction)(static_cast<int32_t>(button) - static_cast<int32_t>(Ds4Button::R_Stick_Up)), joystickThreshold);

	return false;
}
