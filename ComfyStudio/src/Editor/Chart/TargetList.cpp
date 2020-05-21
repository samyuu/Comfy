#include "TargetList.h"
#include <algorithm>

namespace Comfy::Studio::Editor
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
		i64 insertionIndex;
		for (insertionIndex = 0; insertionIndex < Count(); insertionIndex++)
		{
			if (collection[insertionIndex].Tick >= tick)
				break;
		}

		collection.emplace(collection.begin() + insertionIndex, tick, type);
		SetTargetSyncFlagsAround(insertionIndex);
	}

	void TargetList::Remove(i64 index)
	{
		if (index < 0 || index >= Count())
			return;

		collection.erase(begin() + index);
		SetTargetSyncFlagsAround(index);
	}

	void TargetList::Remove(TimelineTick tick, TargetType type)
	{
		for (i64 i = 0; i < Count(); i++)
		{
			const TimelineTarget& target = collection[i];

			if (target.Tick == tick && target.Tick == type)
			{
				Remove(i);
				break;
			}
		}
	}

	i64 TargetList::FindIndex(TimelineTick tick, TargetType type)
	{
		for (i64 i = 0; i < Count(); i++)
		{
			const TimelineTarget& target = collection[i];

			if (target.Tick == tick && target.Type == type)
				return i;
		}

		return -1;
	}

	i64 TargetList::Count()
	{
		return collection.size();
	}

	void TargetList::SetTargetSyncFlagsAround(i64 index)
	{
		SetTargetSyncFlags(index - TargetType_Max, index + TargetType_Max);
	}

	void TargetList::SetTargetSyncFlags(i64 start, i64 end)
	{
		if (start < 0)
			start = 0;

		if (end < 0 || end > Count())
			end = Count();

		for (i64 i = start; i < end; i++)
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
