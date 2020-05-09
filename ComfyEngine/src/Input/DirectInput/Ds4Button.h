#pragma once
#include "Types.h"

namespace Comfy::Input
{
	enum class Direction : i32
	{
		Up,
		Right,
		Down,
		Left
	};

	enum class Ds4Button : i32
	{
		Square = 0,
		Cross = 1,
		Circle = 2,
		Triangle = 3,

		L1 = 4,
		R2 = 5,

		L_Trigger = 6,
		R_Trigger = 7,

		Share = 8,
		Options = 9,

		L3 = 10,
		R3 = 11,

		PS = 12,
		Touch = 13,

		DPad_Up = 14,
		DPad_Right = 15,
		DPad_Down = 16,
		DPad_Left = 17,

		L_Stick_Up = 18,
		L_Stick_Right = 19,
		L_Stick_Down = 20,
		L_Stick_Left = 21,

		R_Stick_Up = 22,
		R_Stick_Right = 23,
		R_Stick_Down = 24,
		R_Stick_Left = 25,

		Count
	};
}
