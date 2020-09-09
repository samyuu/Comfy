#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Input/Input.h"
#include "SortedTargetList.h"

namespace Comfy::Studio::Editor
{
	namespace KeyBindings
	{
		constexpr Input::KeyCode Undo = Input::KeyCode_Z;
		constexpr Input::KeyCode Redo = Input::KeyCode_Y;

		constexpr Input::KeyCode Copy = Input::KeyCode_C;
		constexpr Input::KeyCode Paste = Input::KeyCode_V;
		constexpr Input::KeyCode Cut = Input::KeyCode_X;

		constexpr Input::KeyCode MoveCursorLeft = Input::KeyCode_Left;
		constexpr Input::KeyCode MoveCursorRight = Input::KeyCode_Right;

		constexpr Input::KeyCode IncreaseGridPrecision = Input::KeyCode_Down;
		constexpr Input::KeyCode DecreaseGridPrecision = Input::KeyCode_Up;

		constexpr Input::KeyCode RangeSelection = Input::KeyCode_Tab;
		constexpr Input::KeyCode DeleteSelection = Input::KeyCode_Delete;

		constexpr Input::KeyCode JumpToPreviousTarget = Input::KeyCode_Q;
		constexpr Input::KeyCode JumpToNextTarget = Input::KeyCode_E;

		constexpr Input::KeyCode TogglePlayback = Input::KeyCode_Space;

		constexpr std::array TargetPlacements =
		{
			std::make_pair(ButtonType::Triangle, Input::KeyCode_W),
			std::make_pair(ButtonType::Square, Input::KeyCode_A),
			std::make_pair(ButtonType::Cross, Input::KeyCode_S),
			std::make_pair(ButtonType::Circle, Input::KeyCode_D),
			std::make_pair(ButtonType::SlideL, Input::KeyCode_Q),
			std::make_pair(ButtonType::SlideR, Input::KeyCode_E),

			std::make_pair(ButtonType::Triangle, Input::KeyCode_I),
			std::make_pair(ButtonType::Square, Input::KeyCode_J),
			std::make_pair(ButtonType::Cross, Input::KeyCode_K),
			std::make_pair(ButtonType::Circle, Input::KeyCode_L),
			std::make_pair(ButtonType::SlideL, Input::KeyCode_U),
			std::make_pair(ButtonType::SlideR, Input::KeyCode_O),
		};
	}
}
