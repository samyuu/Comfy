#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "SortedTargetList.h"
#include "RenderWindow/Tools/TargetTool.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	namespace KeyBindings
	{
		constexpr Input::KeyCode Undo = Input::KeyCode_Z;
		constexpr Input::KeyCode Redo = Input::KeyCode_Y;

		constexpr Input::KeyCode Cut = Input::KeyCode_X;
		constexpr Input::KeyCode Copy = Input::KeyCode_C;
		constexpr Input::KeyCode Paste = Input::KeyCode_V;

		constexpr Input::KeyCode MoveCursorLeft = Input::KeyCode_Left;
		constexpr Input::KeyCode MoveCursorRight = Input::KeyCode_Right;

		constexpr Input::KeyCode IncreaseGridPrecision = Input::KeyCode_Down;
		constexpr Input::KeyCode DecreaseGridPrecision = Input::KeyCode_Up;

		constexpr Input::KeyCode RangeSelection = Input::KeyCode_Tab;
		constexpr Input::KeyCode DeleteSelection = Input::KeyCode_Delete;

		constexpr Input::KeyCode IncreasePlaybackSpeed = Input::KeyCode_OEMPeriod;
		constexpr Input::KeyCode DecreasePlaybackSpeed = Input::KeyCode_OEMComma;

		constexpr Input::KeyCode ToggleMetronome = Input::KeyCode_M;

		constexpr Input::KeyCode ToggleTargetHolds = Input::KeyCode_F;

		constexpr Input::KeyCode JumpToPreviousTarget = Input::KeyCode_Q;
		constexpr Input::KeyCode JumpToNextTarget = Input::KeyCode_E;

		constexpr Input::KeyCode TogglePlayback = Input::KeyCode_Space;

		constexpr Input::KeyCode StartPlaytestFromStart = Input::KeyCode_F5;
		constexpr Input::KeyCode StartPlaytestFromCursor = Input::KeyCode_F6;

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

		constexpr std::array<Input::KeyCode, EnumCount<TargetToolType>()> TargetToolTypes =
		{
			Input::KeyCode_1,
			Input::KeyCode_2,
		};

		constexpr std::array PositionToolMoveStep =
		{
			std::make_pair(Input::KeyCode_W, vec2(+0.0f, -1.0f)),
			std::make_pair(Input::KeyCode_A, vec2(-1.0f, +0.0f)),
			std::make_pair(Input::KeyCode_S, vec2(+0.0f, +1.0f)),
			std::make_pair(Input::KeyCode_D, vec2(+1.0f, +0.0f)),

			std::make_pair(Input::KeyCode_Up, vec2(+0.0f, -1.0f)),
			std::make_pair(Input::KeyCode_Left, vec2(-1.0f, +0.0f)),
			std::make_pair(Input::KeyCode_Down, vec2(+0.0f, +1.0f)),
			std::make_pair(Input::KeyCode_Right, vec2(+1.0f, +0.0f)),
		};

		constexpr Input::KeyCode PositionToolFlipHorizontal = Input::KeyCode_H;
		constexpr Input::KeyCode PositionToolFlipVertical = Input::KeyCode_J;
		constexpr Input::KeyCode PositionToolPositionInRow = Input::KeyCode_U;
		constexpr Input::KeyCode PositionToolInterpolateLinear = Input::KeyCode_I;
		constexpr Input::KeyCode PositionToolInterpolateCircular = Input::KeyCode_O;

		constexpr Input::KeyCode PathToolInvertFrequencies = Input::KeyCode_R;
		constexpr Input::KeyCode PathToolInterpolateAnglesClockwise = Input::KeyCode_U;
		constexpr Input::KeyCode PathToolInterpolateAnglesCounterclockwise = Input::KeyCode_I;
		constexpr Input::KeyCode PathToolInterpolateDistances = Input::KeyCode_O;

		constexpr Input::KeyCode PathToolApplyAngleIncrementsPositive = Input::KeyCode_F;
		constexpr Input::KeyCode PathToolApplyAngleIncrementsNegative = Input::KeyCode_V;
	}
}
