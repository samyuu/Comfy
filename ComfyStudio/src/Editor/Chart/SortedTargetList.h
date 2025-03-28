#pragma once
#include "Types.h"
#include "BeatTick.h"
#include "Time/TimeSpan.h"
#include <unordered_map>

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

	using ButtonTypeFlags = u8;
	enum ButtonTypeFlagsEnum : ButtonTypeFlags
	{
		ButtonTypeFlags_None = 0,
		ButtonTypeFlags_Triangle = 1 << static_cast<u8>(ButtonType::Triangle),
		ButtonTypeFlags_Square = 1 << static_cast<u8>(ButtonType::Square),
		ButtonTypeFlags_Cross = 1 << static_cast<u8>(ButtonType::Cross),
		ButtonTypeFlags_Circle = 1 << static_cast<u8>(ButtonType::Circle),
		ButtonTypeFlags_SlideL = 1 << static_cast<u8>(ButtonType::SlideL),
		ButtonTypeFlags_SlideR = 1 << static_cast<u8>(ButtonType::SlideR),

		ButtonTypeFlags_NormalAll = (ButtonTypeFlags_Triangle | ButtonTypeFlags_Square | ButtonTypeFlags_Cross | ButtonTypeFlags_Circle),
		ButtonTypeFlags_SlideAll = (ButtonTypeFlags_SlideL | ButtonTypeFlags_SlideR),
		ButtonTypeFlags_All = (ButtonTypeFlags_NormalAll | ButtonTypeFlags_SlideAll),
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

		TargetPropertyFlags_PositionXY = (TargetPropertyFlags_PositionX | TargetPropertyFlags_PositionY),
		TargetPropertyFlags_All = (TargetPropertyFlags_PositionXY | TargetPropertyFlags_Angle | TargetPropertyFlags_Frequency | TargetPropertyFlags_Amplitude | TargetPropertyFlags_Distance),
	};

	constexpr bool IsSlideButtonType(ButtonType type)
	{
		return (type == ButtonType::SlideL || type == ButtonType::SlideR);
	}

	constexpr ButtonType FlipSlideButtonType(ButtonType type)
	{
		switch (type)
		{
		case ButtonType::SlideL: return ButtonType::SlideR;
		case ButtonType::SlideR: return ButtonType::SlideL;
		default: return type;
		}
	}

	constexpr ButtonType MirrorButtonType(ButtonType type)
	{
		switch (type)
		{
		case ButtonType::Triangle: return ButtonType::Circle;
		case ButtonType::Square: return ButtonType::Cross;
		case ButtonType::Cross: return ButtonType::Square;
		case ButtonType::Circle: return ButtonType::Triangle;
		case ButtonType::SlideL: return ButtonType::SlideR;
		case ButtonType::SlideR: return ButtonType::SlideL;
		default: return type;
		}
	}

	constexpr ButtonTypeFlags ButtonTypeToButtonTypeFlags(ButtonType type)
	{
		return static_cast<ButtonTypeFlags>(1 << static_cast<u32>(type));
	}

	constexpr i32 ButtonTypeFlagsBitCount(ButtonTypeFlags types)
	{
		i32 bitCount = 0;
		for (u8 typeIndex = 0; typeIndex < static_cast<u8>(EnumCount<ButtonType>()); typeIndex++)
			bitCount += static_cast<bool>(types & (1 << typeIndex));
		return bitCount;
	}

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

	constexpr TimeSpan MaxTargetHoldDuration = TimeSpan::FromSeconds(5.0);

	struct TargetFlags
	{
		// NOTE: Use u16 instead of u32 to allow for implicit signed int conversion

		// NOTE: Publicly set flags:
		u16 HasProperties : 1;
		u16 IsHold : 1;
		u16 IsChain : 1;
		u16 IsChance : 1;

		// NOTE: Internally set flags:
		u16 IsSync : 1;
		u16 IsChainStart : 1;
		u16 IsChainEnd : 1;
		u16 /* Reserved */ : 1;

		u16 IndexWithinSyncPair : 4;
		u16 SyncPairCount : 4;

		u16 SameTypeSyncIndex : 4;
		u16 SameTypeSyncCount : 4;

		u16 /* Reserved */ : 8;
	};

	static_assert(sizeof(TargetFlags) == sizeof(u32));

	// NOTE: Stable ID to be used by undoable commands instead of indices
	enum class TimelineTargetID : u32 { Null = 0 };

	struct TimelineTarget
	{
		TimelineTarget() = default;
		TimelineTarget(BeatTick tick, ButtonType type) : Tick(tick), Type(type) {}

		BeatTick Tick = {};
		ButtonType Type = {};
		bool IsSelected = false;
		TargetFlags Flags = {};
		TargetProperties Properties = {};
		TimelineTargetID ID = {};
	};

	static_assert(sizeof(TimelineTarget) == 40);

	class SortedTargetList : NonCopyable
	{
	public:
		// NOTE: Should be called and assigned the very first time a target is created but never overwriten afterwards
		static TimelineTargetID GetNextUniqueID();

	public:
		SortedTargetList();
		~SortedTargetList() = default;

	public:
		COMFY_NODISCARD TimelineTargetID Add(TimelineTarget newTarget);

		void Remove(TimelineTarget target);
		void RemoveAt(i32 index);

		i32 FindIndex(BeatTick tick) const;
		i32 FindIndex(BeatTick tick, ButtonType type) const;
		i32 FindIndex(TimelineTargetID id) const;

		void Clear();

		void ExplicitlyUpdateFlagsAndSortEverything();
		void ExplicitlyUpdateFlagsAndSortIndexRange(i32 startIndex, i32 endIndex);

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

		void operator=(std::vector<TimelineTarget>&& newTargets);

		const std::vector<TimelineTarget>& GetRawView() const { return targets; }

	private:
		size_t FindSortedInsertionIndex(BeatTick tick, ButtonType type) const;

		void UpdateTargetInternalFlagsAround(i32 index);
		void UpdateTargetInternalFlagsInRange(i32 startIndex = -1, i32 endIndex = -1);

		i32 FloorIndexToSyncPairStart(i32 index) const;
		i32 CeilIndexToSyncPairEnd(i32 index) const;
		i32 CountConsecutiveSyncTargets(i32 startIndex) const;
		i32 CountConsecutiveSameTypeSyncTargets(i32 startIndex) const;

		void UpdateSyncPairFlagsInRange(i32 startIndex, i32 endIndex);

		void UpdateChainFlagsInRange(i32 startIndex, i32 endIndex);
		void UpdateChainFlagsForDirection(i32 startIndex, i32 endIndex, ButtonType slideDirection);

	private:
		std::vector<TimelineTarget> targets;
		std::vector<TimelineTarget*> slideTargetBuffer;

		std::unordered_map<TimelineTargetID, i32> idToIndexMap;
	};
}
