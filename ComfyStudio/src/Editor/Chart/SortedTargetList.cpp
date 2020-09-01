#include "SortedTargetList.h"
#include <algorithm>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr u64 GetTargetSortWeight(const TimelineTick tick, const ButtonType type)
		{
			static_assert(sizeof(tick) == sizeof(i32));
			static_assert(sizeof(type) == sizeof(u8));

			u64 totalWeight = 0;
			totalWeight |= (static_cast<u64>(tick.Ticks()) << 32);
			totalWeight |= (static_cast<u64>(type) << 0);
			return totalWeight;
		}

		constexpr u64 GetTargetSortWeight(const TimelineTarget& target)
		{
			return GetTargetSortWeight(target.Tick, target.Type);
		}
	}

	SortedTargetList::SortedTargetList()
	{
		constexpr auto reasonableInitialCapacity = 2000;
		targets.reserve(reasonableInitialCapacity);
	}

	void SortedTargetList::Add(TimelineTick tick, ButtonType type)
	{
#if 1 // NOTE: Sorted insertion, appears to be much faster
		{
			const auto insertionIndex = FindSortedInsertionIndex(tick, type);
			targets.emplace(targets.begin() + insertionIndex, tick, type);

			UpdateTargetSyncFlagsAround(insertionIndex);
		}
#else // NOTE: However a post emplace sort is useful for error checking
		{
			targets.emplace_back(tick, type);
			std::sort(targets.begin(), targets.end(), [&](const auto& a, const auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); });

			UpdateTargetSyncFlags();
		}
#endif

		assert(std::is_sorted(targets.begin(), targets.end(), [&](const auto& a, const auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); }));
	}

	void SortedTargetList::RemoveAt(i64 index)
	{
		if (!InBounds(static_cast<size_t>(index), targets))
			return;

		targets.erase(begin() + index);
		UpdateTargetSyncFlagsAround(index);
	}

	void SortedTargetList::Remove(TimelineTick tick, ButtonType type)
	{
		RemoveAt(FindIndex(tick, type));
	}

	i64 SortedTargetList::FindIndex(TimelineTick tick, ButtonType type) const
	{
		// TODO: Binary search
		const auto foundIndex = FindIndexOf(targets, [&](const auto& target) { return (target.Tick == tick) && (target.Type == type); });
		return InBounds(foundIndex, targets) ? static_cast<i64>(foundIndex) : -1;
	}

	void SortedTargetList::Clear()
	{
		targets.clear();
	}

	size_t SortedTargetList::FindSortedInsertionIndex(TimelineTick tick, ButtonType type) const
	{
		const auto inputSortWeight = GetTargetSortWeight(tick, type);

		size_t insertionIndex;
		for (insertionIndex = 0; insertionIndex < targets.size(); insertionIndex++)
		{
			if (inputSortWeight < GetTargetSortWeight(targets[insertionIndex]))
				break;
		}

		return insertionIndex;
	}

	void SortedTargetList::UpdateTargetSyncFlagsAround(i64 index)
	{
		constexpr auto surroundingTargetsToCheck = static_cast<i64>(EnumCount<ButtonType>());

		UpdateTargetSyncFlags(
			index - surroundingTargetsToCheck,
			index + surroundingTargetsToCheck);
	}

	void SortedTargetList::UpdateTargetSyncFlags(i64 start, i64 end)
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

			target.Flags.IsSync = (prevTarget != nullptr && prevTarget->Tick == target.Tick) || (nextTarget != nullptr && nextTarget->Tick == target.Tick);
		}

		UpdateSyncPairIndexFlags();
	}

	size_t SortedTargetList::GetSyncPairCountAt(size_t targetStartIndex) const
	{
		for (size_t i = targetStartIndex; i < targets.size(); i++)
		{
			const auto& target = targets[i];
			const auto* nextTarget = IndexOrNull(i + 1, targets);

			if (nextTarget == nullptr || target.Tick != nextTarget->Tick)
				return (i - targetStartIndex) + 1;
		}

		return 1;
	}

	void SortedTargetList::UpdateSyncPairIndexFlags()
	{
		for (size_t i = 0; i < targets.size();)
		{
			const auto pairCount = GetSyncPairCountAt(i);
			assert(pairCount > 0);

			for (size_t pairIndex = 0; pairIndex < pairCount; pairIndex++)
			{
				auto& target = targets[i++];
				target.Flags.IndexWithinSyncPair = pairIndex;
				target.Flags.SyncPairCount = pairCount;
			}
		}
	}
}
