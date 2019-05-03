#pragma once
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include "DirectInput.h"
#include "Ds4Button.h"

inline glm::vec2 GetDirection(float degrees)
{
	float radians = glm::radians(degrees);
	return glm::vec2(cos(radians), sin(radians));
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

	BYTE Buttons[DS4_BUTTON_MAX];

	Dpad Dpad;
	Joystick LeftStick;
	Joystick RightStick;
	Trigger LeftTrigger;
	Trigger RightTrigger;
};
