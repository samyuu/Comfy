#pragma once
#include "TimelineTick.h"
#include <set>
#include <glm/vec2.hpp>

namespace Editor
{
	enum TargetType : int16_t
	{
		TargetType_Sankaku,
		TargetType_Shikaku,
		TargetType_Batsu,
		TargetType_Maru,
		TargetType_SlideL,
		TargetType_SlideR,
		TargetType_Max,
	};

	enum TargetFlags : int16_t
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