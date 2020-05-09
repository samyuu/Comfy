#include "DualShock4.h"

namespace Comfy::Input
{
	DualShock4* DualShock4::instance = nullptr;

	DualShock4::DualShock4()
	{
	}

	DualShock4::~DualShock4()
	{
		DI_Dispose();
	}

	bool DualShock4::Initialize()
	{
		HRESULT result = NULL;

		for (const auto& guid : GUID_Ds4)
		{
			result = DI_CreateDevice(guid);

			if (!FAILED(result))
				break;
		}

		if (directInputdevice == nullptr)
			return false;

		if (FAILED(result = DI_SetDataFormat(&c_dfDIJoystick2)))
			return false;

		result = DI_Acquire();

		return true;
	}

	bool DualShock4::PollInput()
	{
		lastState = currentState;

		HRESULT result;
		result = DI_Poll();
		result = DI_GetDeviceState(sizeof(DIJOYSTATE2), &currentState.DI_JoyState);

		if (result != DI_OK)
			return false;

		UpdateInternalDs4State(currentState);

		for (i32 button = 0; button < static_cast<i32>(Ds4Button::Count); button++)
			currentState.Buttons[button] = GetButtonState(currentState, static_cast<Ds4Button>(button));

		return true;
	}

	float DualShock4::NormalizeTrigger(long value) const
	{
		return static_cast<float>(value) / std::numeric_limits<u16>::max();
	}

	float DualShock4::NormalizeStick(long value) const
	{
		return static_cast<float>(value) / std::numeric_limits<u16>::max() * 2.0f - 1.0f;
	}

	Joystick DualShock4::NormalizeStick(long x, long y) const
	{
		return Joystick(NormalizeStick(x), NormalizeStick(y));
	}

	void DualShock4::UpdateInternalDs4State(Ds4State& state)
	{
		if (state.Dpad.IsDown = (state.DI_JoyState.rgdwPOV[0] != -1))
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

	bool DualShock4::Instance_IsDown(Ds4Button button) const
	{
		return currentState.Buttons[static_cast<i32>(button)];
	}

	bool DualShock4::Instance_IsUp(Ds4Button button) const
	{
		return !Instance_IsDown(button);
	}

	bool DualShock4::Instance_IsTapped(Ds4Button button) const
	{
		return Instance_IsDown(button) && Instance_WasUp(button);
	}

	bool DualShock4::Instance_IsReleased(Ds4Button button) const
	{
		return Instance_IsUp(button) && Instance_WasDown(button);
	}

	bool DualShock4::Instance_WasDown(Ds4Button button) const
	{
		return lastState.Buttons[static_cast<i32>(button)];
	}

	bool DualShock4::Instance_WasUp(Ds4Button button) const
	{
		return !Instance_WasDown(button);
	}

	Joystick DualShock4::GetLeftStick() const
	{
		return currentState.LeftStick;
	}

	Joystick DualShock4::GetRightStick() const
	{
		return currentState.RightStick;
	}

	Joystick DualShock4::GetDpad() const
	{
		return currentState.Dpad.Stick;
	}

	bool DualShock4::TryInitializeInstance()
	{
		if (GetInstanceInitialized())
			return true;

		if (!DirectInputInitialized())
			return false;

		DualShock4 *dualShock4 = new DualShock4();

		const bool success = dualShock4->Initialize();
		instance = success ? dualShock4 : nullptr;

		if (!success)
			delete dualShock4;

		return success;
	}

	bool DualShock4::MatchesDirection(Joystick joystick, Direction directionEnum, float threshold) const
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

	bool DualShock4::GetButtonState(Ds4State& state, Ds4Button button) const
	{
		if (button >= Ds4Button::Square && button <= Ds4Button::Touch)
			return state.DI_JoyState.rgbButtons[static_cast<i32>(button)];

		if (button >= Ds4Button::DPad_Up && button <= Ds4Button::DPad_Left)
			return state.Dpad.IsDown ? MatchesDirection(state.Dpad.Stick, static_cast<Direction>(static_cast<i32>(button) - static_cast<i32>(Ds4Button::DPad_Up)), dpadThreshold) : false;

		if (button >= Ds4Button::L_Stick_Up && button <= Ds4Button::L_Stick_Left)
			return MatchesDirection(state.LeftStick, (Direction)(static_cast<i32>(button) - static_cast<i32>(Ds4Button::L_Stick_Up)), joystickThreshold);

		if (button >= Ds4Button::R_Stick_Up && button <= Ds4Button::R_Stick_Left)
			return MatchesDirection(state.RightStick, (Direction)(static_cast<i32>(button) - static_cast<i32>(Ds4Button::R_Stick_Up)), joystickThreshold);

		return false;
	}
}
