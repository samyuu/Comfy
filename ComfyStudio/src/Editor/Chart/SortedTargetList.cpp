#include "SortedTargetList.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	SortedTargetList::SortedTargetList()
	{
		constexpr auto reasonableInitialCapacity = 2000;
		targets.reserve(reasonableInitialCapacity);
	}

	void SortedTargetList::Add(TimelineTick tick, TargetType type)
	{
		size_t insertionIndex;
		for (insertionIndex = 0; insertionIndex < targets.size(); insertionIndex++)
		{
			if (targets[insertionIndex].Tick >= tick)
				break;
		}

		targets.emplace(targets.begin() + insertionIndex, tick, type);
		SetTargetSyncFlagsAround(insertionIndex);
		
		assert(std::is_sorted(targets.begin(), targets.end(), [&](const auto& targetA, const auto& targetB) 
		{ 
			return (targetA.Tick < targetB.Tick) && (targetA.Type < targetB.Type);
		}));
	}

	void SortedTargetList::RemoveAt(i64 index)
	{
		if (!InBounds(static_cast<size_t>(index), targets))
			return;

		targets.erase(begin() + index);
		SetTargetSyncFlagsAround(index);
	}

	void SortedTargetList::Remove(TimelineTick tick, TargetType type)
	{
		RemoveAt(FindIndex(tick, type));
	}

	i64 SortedTargetList::FindIndex(TimelineTick tick, TargetType type)
	{
		const auto foundIndex = FindIndexOf(targets, [&](const auto& target) { return (target.Tick == tick) && (target.Type == type); });
		return InBounds(foundIndex, targets) ? static_cast<i64>(foundIndex) : -1;
	}

	void SortedTargetList::SetTargetSyncFlagsAround(i64 index)
	{
		SetTargetSyncFlags(index - TargetType_Max, index + TargetType_Max);
	}

	void SortedTargetList::SetTargetSyncFlags(i64 start, i64 end)
	{
		if (start < 0)
			start = 0;

		const auto targetCount = static_cast<i64>(targets.size());

		if (end < 0 || end > targetCount)
			end = targetCount;

		for (i64 i = start; i < end; i++)
		{
			auto& target = targets[i];
			const auto* nextTarget = (i + 1 < targetCount) ? &targets[i + 1] : nullptr;
			const auto* prevTarget = (i - 1 >= 0) ? &targets[i - 1] : nullptr;

			target.Flags &= ~TargetFlags_Sync;

			if ((prevTarget != nullptr && prevTarget->Tick == target.Tick) || (nextTarget != nullptr && nextTarget->Tick == target.Tick))
				target.Flags |= TargetFlags_Sync;
		}
	}
}
