#pragma once
#include "Timeline/TimelineTick.h"
#include "Types.h"
#include "Core/CoreTypes.h"

namespace Editor
{
	typedef int16_t TargetType;
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

	typedef int16_t TargetFlags;
	enum TargetFlags_Enum : TargetFlags
	{
		// Subject to change
		TargetFlags_None = 0,
		TargetFlags_Hold = 1 << 0,
		TargetFlags_Sync = 1 << 1,
		TargetFlags_Chain = 1 << 2,
		TargetFlags_ChainStart = 1 << 3,
		TargetFlags_ChainHit = 1 << 4,
	};

	struct TargetProperties
	{
		glm::vec2 Position;
		float Angle;
		short Frequency;
		short Amplitude;
		float Distance;
	};

	struct TimelineTarget
	{
		// Members:
		// --------
		TimelineTick Tick;
		TargetType Type;
		TargetFlags Flags;
		// TargetProperties Properties;
		// --------

		// Constructors:
		// -------------
		TimelineTarget();
		TimelineTarget(TimelineTick, TargetType);
		TimelineTarget(const TimelineTarget&);
		// -------------

		// Operators:
		// ----------
		inline bool operator== (const TimelineTarget &other) const { return (Type == other.Type) && (Tick == other.Tick); };
		inline bool operator< (const TimelineTarget &other) const { return Tick < other.Tick; };
		inline bool operator> (const TimelineTarget &other) const { return Tick > other.Tick; };
		// ----------
	};

	using TargetCollection = Vector<TimelineTarget>;
	using TargetIterator = TargetCollection::iterator;
	using ConstTargetIterator = TargetCollection::const_iterator;

	class TargetList
	{
	public:
		// Constructors:
		// -------------
		TargetList();

		// Access Methods:
		// ---------------
		void Add(TimelineTick, TargetType);
		void Remove(int64_t index);
		void Remove(TimelineTick, TargetType);
		int64_t FindIndex(TimelineTick, TargetType);
		int64_t Count();
		// ---------------

		// Iterators:
		// ----------
		TargetIterator begin() { return collection.begin(); }
		TargetIterator end() { return collection.end(); }
		ConstTargetIterator begin() const { return collection.begin(); }
		ConstTargetIterator end() const { return collection.end(); }
		ConstTargetIterator cbegin() const { return collection.cbegin(); }
		ConstTargetIterator cend() const { return collection.cend(); }
		// ----------

	private:
		TargetCollection collection;

		void SetTargetSyncFlagsAround(int64_t index);
		void SetTargetSyncFlags(int64_t start = -1, int64_t end = -1);
	};
}