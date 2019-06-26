#include "TargetList.h"
#include "Misc/BitFlagsHelper.h"
#include <algorithm>

namespace Editor
{
	TimelineTarget::TimelineTarget() : Tick(0), Type(static_cast<TargetType>(0)), Flags(TargetFlags_None)
	{
	}

	TimelineTarget::TimelineTarget(TimelineTick tick, TargetType type) : Tick(tick), Type(type), Flags(TargetFlags_None)
	{
	}

	TimelineTarget::TimelineTarget(const TimelineTarget& other) : Tick(other.Tick), Type(other.Type), Flags(other.Flags)
	{
	}

	TargetList::TargetList() : collection()
	{
	}

	void TargetList::Add(TimelineTick tick, TargetType type)
	{
		int64_t insertionIndex;
		for (insertionIndex = 0; insertionIndex < Count(); insertionIndex++)
		{
			if (collection[insertionIndex].Tick >= tick)
				break;
		}

		collection.emplace(collection.begin() + insertionIndex, tick, type);
		SetTargetSyncFlagsAround(insertionIndex);
	}

	void TargetList::Remove(int64_t index)
	{
		if (index < 0 || index >= Count())
			return;

		collection.erase(begin() + index);
		SetTargetSyncFlagsAround(index);
	}

	void TargetList::Remove(TimelineTick tick, TargetType type)
	{
		for (int64_t i = 0; i < Count(); i++)
		{
			const TimelineTarget& target = collection[i];

			if (target.Tick == tick && target.Tick == type)
			{
				Remove(i);
				break;
			}
		}
	}

	int64_t TargetList::FindIndex(TimelineTick tick, TargetType type)
	{
		for (int64_t i = 0; i < Count(); i++)
		{
			const TimelineTarget& target = collection[i];

			if (target.Tick == tick && target.Type == type)
				return i;
		}

		return -1;
	}

	inline int64_t TargetList::Count()
	{
		return collection.size();
	}

	void TargetList::SetTargetSyncFlagsAround(int64_t index)
	{
		SetTargetSyncFlags(index - TargetType_Max, index + TargetType_Max);
	}

	void TargetList::SetTargetSyncFlags(int64_t start, int64_t end)
	{
		if (start < 0)
			start = 0;

		if (end < 0 || end > Count())
			end = Count();

		for (int64_t i = start; i < end; i++)
		{
			TimelineTarget* target = &collection[i];
			TimelineTarget* nextTarget = (i + 1 < Count()) ? &collection[i + 1] : nullptr;
			TimelineTarget* prevTarget = (i - 1 >= 0) ? &collection[i - 1] : nullptr;

			target->Flags &= ~TargetFlags_Sync;

			if ((prevTarget && prevTarget->Tick == target->Tick) || (nextTarget && nextTarget->Tick == target->Tick))
				target->Flags |= TargetFlags_Sync;
		}
	}
}