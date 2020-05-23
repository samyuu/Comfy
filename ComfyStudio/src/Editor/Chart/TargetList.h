#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Timeline/TimelineTick.h"

namespace Comfy::Studio::Editor
{
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

	struct TargetProperties
	{
		vec2 Position;
		float Angle;
		short Frequency;
		short Amplitude;
		float Distance;
	};

	struct TimelineTarget
	{
	public:
		TimelineTarget() = default;
		TimelineTarget(TimelineTick tick, TargetType type) : Tick(tick), Type(type), Flags(TargetFlags_None) {}

	public:
		TimelineTick Tick = {};
		TargetType Type = {};
		TargetFlags Flags = TargetFlags_None;

	public:
		bool operator==(const TimelineTarget &other) const { return (Type == other.Type) && (Tick == other.Tick); }
		bool operator<(const TimelineTarget &other) const { return Tick < other.Tick; }
		bool operator>(const TimelineTarget &other) const { return Tick > other.Tick; }
	};

	class TargetList
	{
	public:
		TargetList() = default;
		~TargetList() = default;

	public:
		void Add(TimelineTick, TargetType);
		void Remove(i64 index);
		void Remove(TimelineTick, TargetType);
		i64 FindIndex(TimelineTick, TargetType);
		i64 Count();

	public:
		auto begin() { return collection.begin(); }
		auto end() { return collection.end(); }
		auto begin() const { return collection.begin(); }
		auto end() const { return collection.end(); }
		auto cbegin() const { return collection.cbegin(); }
		auto cend() const { return collection.cend(); }

	private:
		std::vector<TimelineTarget> collection;

		void SetTargetSyncFlagsAround(i64 index);
		void SetTargetSyncFlags(i64 start = -1, i64 end = -1);
	};
}
