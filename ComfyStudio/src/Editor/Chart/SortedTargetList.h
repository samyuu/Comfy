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
		i32 Frequency;
		f32 Amplitude;
		f32 Distance;
	};

	struct TargetFlags
	{
		// NOTE: Publicly set flags:
		u16 HasProperties : 1;
		u16 IsHold : 1;
		u16 IsChain : 1;
		u16 IsChance : 1;

		// NOTE: Internally set flags:
		u16 IsSync : 1;
		u16 IsChainStart : 1;
		u16 IsChainEnd : 1;
		u16 /* SyncPairHasDuplicate */ : 1;
		u16 IndexWithinSyncPair : 4;
		u16 SyncPairCount : 4;
	};

	static_assert(sizeof(TargetFlags) == sizeof(u16));

	struct TimelineTarget
	{
		TimelineTarget() = default;
		TimelineTarget(TimelineTick tick, ButtonType type) : Tick(tick), Type(type) {}

		TimelineTick Tick = {};
		ButtonType Type = {};
		bool IsSelected = false;
		TargetFlags Flags = {};
		TargetProperties Properties = {};
	};

	class SortedTargetList : NonCopyable
	{
	public:
		SortedTargetList();
		~SortedTargetList() = default;

	public:
		void Add(TimelineTarget newTarget);

		void Remove(TimelineTarget target);
		void RemoveAt(i64 index);

		i64 FindIndex(TimelineTick tick, ButtonType type) const;
		void Clear();

	public:
		auto begin() { return targets.begin(); }
		auto end() { return targets.end(); }
		auto begin() const { return targets.begin(); }
		auto end() const { return targets.end(); }

		auto rbegin() { return targets.rbegin(); }
		auto rend() { return targets.rend(); }
		auto rbegin() const { return targets.rbegin(); }
		auto rend() const { return targets.rend(); }

		size_t size() const { return targets.size(); }

		auto& operator[](size_t index) { return targets[index]; }
		auto& operator[](size_t index) const { return targets[index]; }

	private:
		std::vector<TimelineTarget> targets;

		size_t FindSortedInsertionIndex(TimelineTick tick, ButtonType type) const;

		void UpdateTargetSyncFlagsAround(i64 index);
		void UpdateTargetSyncFlags(i64 start = -1, i64 end = -1);

		size_t GetSyncPairCountAt(size_t targetStartIndex) const;
		void UpdateSyncPairIndexFlags();
	};
}
