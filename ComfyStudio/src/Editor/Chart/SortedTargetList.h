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

	using TargetPropertyType = u8;
	enum TargetPropertyTypeEnum : TargetPropertyType
	{
		TargetPropertyType_PositionX,
		TargetPropertyType_PositionY,
		TargetPropertyType_Angle,
		TargetPropertyType_Frequency,
		TargetPropertyType_Amplitude,
		TargetPropertyType_Distance,
		TargetPropertyType_Count
	};

	using TargetPropertyFlags = u8;
	enum TargetPropertyFlagsEnum : TargetPropertyFlags
	{
		TargetPropertyFlags_None = 0,
		TargetPropertyFlags_PositionX = 1 << TargetPropertyType_PositionX,
		TargetPropertyFlags_PositionY = 1 << TargetPropertyType_PositionY,
		TargetPropertyFlags_Angle = 1 << TargetPropertyType_Angle,
		TargetPropertyFlags_Frequency = 1 << TargetPropertyType_Frequency,
		TargetPropertyFlags_Amplitude = 1 << TargetPropertyType_Amplitude,
		TargetPropertyFlags_Distance = 1 << TargetPropertyType_Distance,
	};

	struct TargetProperties
	{
		vec2 Position;
		f32 Angle;
		f32 Frequency;
		f32 Amplitude;
		f32 Distance;

		auto& operator[](TargetPropertyType type) { assert(type < TargetPropertyType_Count); return reinterpret_cast<f32*>(this)[type]; }
		auto& operator[](TargetPropertyType type) const { assert(type < TargetPropertyType_Count); return reinterpret_cast<const f32*>(this)[type]; }
	};

	static_assert(sizeof(TargetProperties) == (sizeof(f32) * TargetPropertyType_Count));

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
		void RemoveAt(i32 index);

		i32 FindIndex(TimelineTick tick, ButtonType type) const;
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

		void UpdateTargetSyncFlagsAround(i32 index);
		void UpdateTargetSyncFlags(i32 start = -1, i32 end = -1);

		size_t GetSyncPairCountAt(size_t targetStartIndex) const;
		void UpdateSyncPairIndexFlags();
	};
}
