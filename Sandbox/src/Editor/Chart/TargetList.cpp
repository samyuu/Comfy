#include "TargetList.h"

namespace Editor
{
	TimelineTarget::TimelineTarget() : Tick(0), Type(static_cast<TargetType>(0)), Flags(TargetFlags_None)
	{
	}
	
	TimelineTarget::TimelineTarget(TimelineTick tick, TargetType type) : Tick(tick), Type(type), Flags(TargetFlags_None)
	{
	}

	void TargetList::Add(TimelineTick tick, TargetType type)
	{
		collection.emplace(tick, type);
	}

	void TargetList::Remove(TargetIterator iterator)
	{
		if (iterator != collection.end())
			collection.erase(iterator);
	}

	void TargetList::Remove(TimelineTick tick, TargetType type)
	{
		TargetIterator iterator;
		for (iterator = collection.begin(); iterator != collection.end(); ++iterator)
		{
			if (iterator->Tick == tick && iterator->Type == type)
				break;
		}

		Remove(iterator);
	}

	TargetIterator TargetList::Find(TimelineTick tick, TargetType type)
	{
		TargetIterator iterator;
		for (iterator = collection.begin(); iterator != collection.end(); ++iterator)
		{
			if (iterator->Tick == tick && iterator->Type == type)
				return iterator;
		}

		return iterator;
	}
}