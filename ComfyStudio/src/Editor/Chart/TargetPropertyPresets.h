#pragma once
#include "Types.h"
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

	constexpr std::array<const char*, EnumCount<DynamicSyncPreset>()> DynamicSyncPresetNames =
	{
		"Vertical Left",
		"Vertical Right",
		"Horizontal Up",
		"Horizontal Down",
		"Square",
		"Triangle",
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

	void ApplyDynamicSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings);
	void ApplyStaticSyncPresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const StaticSyncPreset& preset);

	u32 FindFirstApplicableDynamicSyncPresetDataForSelectedTargets(const Chart& chart, const DynamicSyncPreset preset, const DynamicSyncPresetSettings& settings, std::array<PresetTargetData, Rules::MaxSyncPairCount>& outPresetTargets);

	enum class SequencePresetType : u8
	{
		Circle,
		BezierPath,
		Count
	};

	enum class SequencePresetButtonType : u8
	{
		SingleLine,
		SameLine,
		Count
	};

	constexpr f32 SequencePresetCircleClockwiseDirection = +1.0f;
	constexpr f32 SequencePresetCircleCounterclockwiseDirection = -1.0f;

	struct SequencePreset
	{
		SequencePresetType Type;
		SequencePresetButtonType ButtonType;
		std::string Name;

		struct CircleData
		{
			BeatTick Duration;
			f32 Radius;
			f32 Direction;
			vec2 Center;
		} Circle;

		struct BezierPathData
		{
			struct Key { vec2 Point, ControlStart, ControlEnd; };
			std::vector<Key> Keys;
		} BezierPath;
	};

	struct SequencePresetSettings
	{
		// TODO: ...
		BeatTick TickOffset;
		bool ApplyFirstTargetTickAsOffset;
	};

	void ApplySequencePresetToSelectedTargets(Undo::UndoManager& undoManager, Chart& chart, const SequencePreset& preset, const SequencePresetSettings& settings);
}
