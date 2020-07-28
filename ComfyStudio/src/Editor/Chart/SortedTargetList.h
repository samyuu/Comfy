#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Timeline/TimelineTick.h"

namespace Comfy::Studio::Editor
{
	// TODO: Remove
	using TargetType = i16;
	enum TargetType_Enum : TargetType
	{
		TargetType_Sankaku,
		TargetType_Shikaku,
		TargetType_Batsu,
		TargetType_Maru,
		TargetType_SlideL,
		TargetType_SlideR,
		TargetType_Max,
	};

	// TODO:
	/*
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
	*/

	// TODO: Remove
	using TargetFlags = i16;
	enum TargetFlags_Enum : TargetFlags
	{
		// NOTE: Subject to change
		TargetFlags_None = 0,
		TargetFlags_Hold = 1 << 0,
		TargetFlags_Sync = 1 << 1,
		TargetFlags_Chain = 1 << 2,
		TargetFlags_ChainStart = 1 << 3,
		TargetFlags_ChainHit = 1 << 4,
	};

	// TODO:
	/*
	struct TargetProperties
	{
		vec2 Position;
		f32 Angle;
		i16 Frequency;
		i16 Amplitude;
		f32 Distance;
	};
	*/

	// TODO:
	/*
	struct TargetFlags
	{
		u16 IsHold : 1;
		u16 IsSync : 1;
		u16 IsChain : 1;
		u16 IsChainStart : 1;
		u16 IsChainEnd : 1;
		// u16 HasProperties : 1;
	};
	*/

	struct TimelineTarget
	{
		TimelineTarget() = default;
		TimelineTarget(TimelineTick tick, TargetType type) : Tick(tick), Type(type), Flags(TargetFlags_None) {}

		TimelineTick Tick;
		// TODO: ButtonType Type;
		TargetType Type;
		TargetFlags Flags;
		// TODO: TargetProperties Properties;
	};

	class SortedTargetList : NonCopyable
	{
	public:
		SortedTargetList();
		~SortedTargetList() = default;

	public:
		void Add(TimelineTick tick, TargetType type);
		void RemoveAt(i64 index);
		void Remove(TimelineTick tick, TargetType type);
		i64 FindIndex(TimelineTick tick, TargetType type);

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

		void SetTargetSyncFlagsAround(i64 index);
		void SetTargetSyncFlags(i64 start = -1, i64 end = -1);
	};
}
