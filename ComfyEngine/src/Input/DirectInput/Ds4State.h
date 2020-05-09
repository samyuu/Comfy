#pragma once
#include "Types.h"
#include "DirectInput.h"
#include "Ds4Button.h"

namespace Comfy::Input
{
	inline constexpr vec2 GetDirection(const float degrees)
	{
		const float radians = glm::radians(degrees);
		return vec2(glm::cos(radians), glm::sin(radians));
	}

	struct Joystick
	{
		FLOAT XAxis, YAxis;

		Joystick();
		Joystick(float xAxis, float yAxis);
	};

	struct Dpad
	{
		BOOL IsDown;
		FLOAT Angle;
		Joystick Stick;
	};

	struct Trigger
	{
		FLOAT Axis;
	};

	struct Ds4State
	{
		DIJOYSTATE2 DI_JoyState;
		BYTE Buttons[static_cast<size_t>(Ds4Button::Count)];

		Dpad Dpad;
		Joystick LeftStick;
		Joystick RightStick;
		Trigger LeftTrigger;
		Trigger RightTrigger;
	};
}
