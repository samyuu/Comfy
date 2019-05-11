#pragma once
#include "TimelineTick.h"
#include <set>

namespace Editor
{
	enum TargetType
	{
		TARGET_SANKAKU,
		TARGET_SHIKAKU,
		TARGET_BATSU,
		TARGET_MARU,
		TARGET_SLIDE_L,
		TARGET_SLIDE_R,
		TARGET_MAX,
	};

	struct TimelineTarget
	{
		// Members:
		// --------
		TimelineTick Tick;
		TargetType Type;
		// bool IsHold, IsSync;
		// --------

		// Constructors:
		// -------------
		TimelineTarget();
		TimelineTarget(TimelineTick, TargetType);
		// -------------

		// Operators:
		// ----------
		inline bool operator== (const TimelineTarget &other) const { return (Type == other.Type) && (Tick == other.Tick); };
		inline bool operator< (const TimelineTarget &other) const { return Tick < other.Tick; };
		inline bool operator> (const TimelineTarget &other) const { return Tick > other.Tick; };
		// ----------
	};

	using TargetMultiset = std::multiset<TimelineTarget>;
	using TargetIterator = TargetMultiset::iterator;
	using ConstTargetIterator = TargetMultiset::const_iterator;

	class TargetList
	{
	public:
		// Access Methods:
		// ---------------
		void Add(TimelineTick, TargetType);
		void Remove(TargetIterator);
		void Remove(TimelineTick, TargetType);
		TargetIterator Find(TimelineTick, TargetType);
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
		TargetMultiset collection;
	};
}