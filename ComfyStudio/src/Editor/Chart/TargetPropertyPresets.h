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

	struct DynamicSyncPresetSettings
	{
		// NOTE: Vertical and Horizontal
		bool SteepAngles = false;
		bool SameDirectionAngles = false;

		// NOTE: Square and Triangle
		bool InsideOutAngles = false;
		bool ElevateBottomRow = false;
	};

	struct PresetTargetData
	{
		ButtonType Type;
		TargetProperties Properties;
	};

	struct StaticSyncPreset
	{
		std::string Name;
		u32 TargetCount;
		std::array<PresetTargetData, Rules::MaxSyncPairCount> Targets;
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

	void ApplyDynamicSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings);
	void ApplyStaticSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const StaticSyncPreset& preset);

	u32 FindFirstApplicableDynamicSyncPresetDataForSelectedTargets(const Chart& chart, const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings, std::array<PresetTargetData, Rules::MaxSyncPairCount>& outPresetTargets);
}
