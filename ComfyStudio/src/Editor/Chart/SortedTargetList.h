#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Timeline/TimelineTick.h"

namespace Comfy::Studio::Editor
{
	enum class ButtonType : u8
	{
		Triangle,
		Square,
		Cross,
		Circle,
		SlideL,
		SlideR,
		Count
	};

	struct TargetProperties
	{
		vec2 Position;
		f32 Angle;
		f32 Frequency;
		f32 Amplitude;
		f32 Distance;
	};

	struct TargetFlags
	{
		u16 IsHold : 1;
		u16 IsSync : 1;
		u16 IsChain : 1;
		u16 IsChainStart : 1;
		u16 IsChainEnd : 1;
		// TODO: Implement a system in which newly placed targets use default properties until manually set otherwise
		// u16 HasProperties : 1;
	};

	struct TimelineTarget
	{
		TimelineTarget() = default;
		TimelineTarget(TimelineTick tick, ButtonType type) : Tick(tick), Type(type) {}

		TimelineTick Tick = {};
		ButtonType Type = {};
		TargetFlags Flags = {};
		
		// TODO: Implement as part of the target render window
		// TargetProperties Properties = {};
	};

	class SortedTargetList : NonCopyable
	{
	public:
		SortedTargetList();
		~SortedTargetList() = default;

	public:
		void Add(TimelineTick tick, ButtonType type);
		void RemoveAt(i64 index);
		void Remove(TimelineTick tick, ButtonType type);
		i64 FindIndex(TimelineTick tick, ButtonType type);

	public:
		auto begin() { return targets.begin(); }
		auto end() { return targets.end(); }
		auto begin() const { return targets.begin(); }
		auto end() const { return targets.end(); }
		auto cbegin() const { return targets.cbegin(); }
		auto cend() const { return targets.cend(); }
		size_t size() const { return targets.size(); }

	private:
		std::vector<TimelineTarget> targets;

		void UpdateTargetSyncFlagsAround(i64 index);
		void UpdateTargetSyncFlags(i64 start = -1, i64 end = -1);
	};
}
