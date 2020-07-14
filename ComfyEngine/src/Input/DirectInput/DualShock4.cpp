#include "DualShock4.h"

namespace Comfy::Input
{
	namespace
	{
		inline vec2 AngleToDircetionVector(float degrees)
		{
			const float radians = glm::radians(degrees);
			return vec2(glm::cos(radians), glm::sin(radians));
		}
	}

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

		for (const auto& guid : GUID_DS4)
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

		for (i32 button = 0; button < static_cast<i32>(DS4Button::Count); button++)
			currentState.Buttons[button] = GetButtonState(currentState, static_cast<DS4Button>(button));

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

	JoystickState DualShock4::NormalizeStick(long x, long y) const
	{
		return JoystickState(NormalizeStick(x), NormalizeStick(y));
	}

	void DualShock4::UpdateInternalDs4State(DS4State& state)
	{
		if (state.Dpad.IsDown = (state.DI_JoyState.rgdwPOV[0] != -1))
		{
			state.Dpad.Angle = (state.DI_JoyState.rgdwPOV[0] / 100.0f);

			const auto directionVector = AngleToDircetionVector(state.Dpad.Angle);
			state.Dpad.Stick = { directionVector.y, -directionVector.x };
		}
		else
		{
			state.Dpad.Angle = 0;
			state.Dpad.Stick = JoystickState();
		}

		state.LeftStick = NormalizeStick(state.DI_JoyState.lX, state.DI_JoyState.lY);
		state.RightStick = NormalizeStick(state.DI_JoyState.lZ, state.DI_JoyState.lRz);

		state.LeftTrigger = { NormalizeTrigger(state.DI_JoyState.lRx) };
		state.RightTrigger = { NormalizeTrigger(state.DI_JoyState.lRy) };
	}

	bool DualShock4::Instance_IsDown(DS4Button button) const
	{
		return currentState.Buttons[static_cast<i32>(button)];
	}

	bool DualShock4::Instance_IsUp(DS4Button button) const
	{
		return !Instance_IsDown(button);
	}

	bool DualShock4::Instance_IsTapped(DS4Button button) const
	{
		return Instance_IsDown(button) && Instance_WasUp(button);
	}

	bool DualShock4::Instance_IsReleased(DS4Button button) const
	{
		return Instance_IsUp(button) && Instance_WasDown(button);
	}

	bool DualShock4::Instance_WasDown(DS4Button button) const
	{
		return lastState.Buttons[static_cast<i32>(button)];
	}

	bool DualShock4::Instance_WasUp(DS4Button button) const
	{
		return !Instance_WasDown(button);
	}

	JoystickState DualShock4::GetLeftStick() const
	{
		return currentState.LeftStick;
	}

	JoystickState DualShock4::GetRightStick() const
	{
		return currentState.RightStick;
	}

	JoystickState DualShock4::GetDpad() const
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

	bool DualShock4::MatchesDirection(JoystickState joystick, DS4Direction directionEnum, float threshold) const
	{
		switch (directionEnum)
		{
		case DS4Direction::Up:
			return joystick.YAxis <= -threshold;

		case DS4Direction::Right:
			return joystick.XAxis >= +threshold;

		case DS4Direction::Down:
			return joystick.YAxis >= +threshold;

		case DS4Direction::Left:
			return joystick.XAxis <= -threshold;

		default:
			return false;
		}
	}

	bool DualShock4::GetButtonState(DS4State& state, DS4Button button) const
	{
		if (button >= DS4Button::Square && button <= DS4Button::Touch)
			return state.DI_JoyState.rgbButtons[static_cast<i32>(button)];

		if (button >= DS4Button::DPad_Up && button <= DS4Button::DPad_Left)
			return state.Dpad.IsDown ? MatchesDirection(state.Dpad.Stick, static_cast<DS4Direction>(static_cast<i32>(button) - static_cast<i32>(DS4Button::DPad_Up)), dpadThreshold) : false;

		if (button >= DS4Button::L_Stick_Up && button <= DS4Button::L_Stick_Left)
			return MatchesDirection(state.LeftStick, (DS4Direction)(static_cast<i32>(button) - static_cast<i32>(DS4Button::L_Stick_Up)), joystickThreshold);

		if (button >= DS4Button::R_Stick_Up && button <= DS4Button::R_Stick_Left)
			return MatchesDirection(state.RightStick, (DS4Direction)(static_cast<i32>(button) - static_cast<i32>(DS4Button::R_Stick_Up)), joystickThreshold);

		return false;
	}
}
