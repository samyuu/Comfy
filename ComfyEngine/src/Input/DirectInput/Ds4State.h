#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "DirectInput.h"
#include "DS4Button.h"

namespace Comfy::Input
{
	struct JoystickState
	{
		float XAxis, YAxis;

		JoystickState() : XAxis(0.0f), YAxis(0.0f) {}
		JoystickState(float xAxis, float yAxis) : XAxis(xAxis), YAxis(yAxis) {}
	};

	struct DPadState
	{
		bool IsDown;
		float Angle;
		JoystickState Stick;
	};

	struct TriggerState
	{
		float Axis;
	};

	struct DS4State
	{
		DIJOYSTATE2 DI_JoyState;
		std::array<bool, EnumCount<DS4Button>()> Buttons;

		DPadState Dpad;
		JoystickState LeftStick;
		JoystickState RightStick;
		TriggerState LeftTrigger;
		TriggerState RightTrigger;
	};
}
