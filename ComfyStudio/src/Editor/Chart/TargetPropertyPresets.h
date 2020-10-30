#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "TargetPropertyRules.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	enum class DynamicSyncPreset : u8
	{
		VerticalLeft,
		VerticalRight,
		HorizontalUp,
		HorizontalDown,
		Square,
		Triangle,
		Count
	};

	struct StaticSyncPreset
	{
		struct Data { ButtonType Type; TargetProperties Properties; };
		static constexpr size_t MaxSyncPairCount = 4;

		std::string Name;
		u32 TargetCount;
		std::array<Data, MaxSyncPairCount> Targets;
	};

	// TODO: Implement as bezier curve points (?)
	/*
	enum class SequencePreset : u8
	{
		LowerLineLeftRight,
		LowerLineRightLeft,

		UpperLineLeftRight,
		UpperLineRightLeft,

		RoundTripLineLowerLeft,
		RoundTripLineLowerRight,

		TwoTimesRoundTripLineLowerLeft,
		TwoTimesRoundTripLineLowerRight,

		VerticalLineUpperLeft,
		VerticalLineUpperRight,

		ArcingLinePeakLeftRight,
		ArcingLinePeakRightLeft,

		ArcingLineDipLeftRight,
		ArcingLineDipRightLeft,

		ParallelogramClockwise,
		ParallelogramCounterclockwise,

		ParallelogramAltClockwise,
		ParallelogramAltCounterclockwise,

		EquilateralTriangleClockwise,
		EquilateralTriangleCounterclockwise,

		SmallCircleClockwise,
		SmallCircleCounterclockwise,

		LargeCircleClockwise,
		LargeCircleCounterclockwise,

		Count
	};
	*/

	void ApplyDynamicSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const DynamicSyncPreset preset);
	void ApplyStaticSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const StaticSyncPreset& preset);
}
