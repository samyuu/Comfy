#include "SortedTargetList.h"
#include <algorithm>
#include <atomic>

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr u64 GetTargetSortWeight(const BeatTick tick, const ButtonType type)
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

		std::atomic<std::underlying_type_t<TimelineTargetID>> GlobalTimelineTargetIDCounter = {};

#if COMFY_DEBUG
		void AssertTargetListOrder(const std::vector<TimelineTarget>& targets)
		{
			assert(std::is_sorted(targets.begin(), targets.end(), [](auto& a, auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); }));
		}

		void AssertTargetListFlags(const std::vector<TimelineTarget>& targets)
		{
			for (i32 targetIndex = 0; targetIndex < static_cast<i32>(targets.size());)
			{
				const i32 remainigTargets = static_cast<i32>(targets.size()) - (targetIndex + 1);
				const auto& firstPairTarget = targets[targetIndex];

				if (firstPairTarget.Flags.SyncPairCount == 0 || (firstPairTarget.Flags.SyncPairCount - 1) > remainigTargets)
					assert(false);

				if (firstPairTarget.Flags.SameTypeSyncCount > firstPairTarget.Flags.SyncPairCount)
					assert(false);

				if (firstPairTarget.Flags.IsSync && (firstPairTarget.Flags.SyncPairCount <= 1))
					assert(false);

				for (i32 withinPairIndex = 0; withinPairIndex < firstPairTarget.Flags.SyncPairCount; withinPairIndex++)
				{
					const auto& otherPairTarget = targets[targetIndex + withinPairIndex];
					if (otherPairTarget.Flags.SyncPairCount != firstPairTarget.Flags.SyncPairCount)
						assert(false);

					if (otherPairTarget.Tick != firstPairTarget.Tick)
						assert(false);

					if (otherPairTarget.Flags.IndexWithinSyncPair != withinPairIndex)
						assert(false);
				}

				for (i32 withinPairIndex = 0; withinPairIndex < firstPairTarget.Flags.SyncPairCount;)
				{
					const auto& firstSameTypeTarget = targets[targetIndex + withinPairIndex];

					const i32 remainigTargetsWithinPair = static_cast<i32>(firstPairTarget.Flags.SyncPairCount - (withinPairIndex + 1));
					if ((firstSameTypeTarget.Flags.SameTypeSyncCount - 1) > remainigTargetsWithinPair)
						assert(false);

					for (i32 withinSameTypePairIndex = 0; withinSameTypePairIndex < firstSameTypeTarget.Flags.SameTypeSyncCount; withinSameTypePairIndex++)
					{
						const auto& otherSameTypeTarget = targets[targetIndex + withinPairIndex + withinSameTypePairIndex];
						if (otherSameTypeTarget.Type != firstSameTypeTarget.Type)
							assert(false);

						if (otherSameTypeTarget.Flags.SameTypeSyncIndex != withinSameTypePairIndex)
							assert(false);
					}

					withinPairIndex += firstSameTypeTarget.Flags.SameTypeSyncCount;
				}

				if (firstPairTarget.Flags.SyncPairCount <= remainigTargets)
				{
					const auto& targetFollowingThisPair = targets[targetIndex + firstPairTarget.Flags.SyncPairCount];
					if (targetFollowingThisPair.Tick == firstPairTarget.Tick)
						assert(false);
				}

				targetIndex += firstPairTarget.Flags.SyncPairCount;
			}
		}

		void AssertTargetIDToIndexMap(const std::vector<TimelineTarget>& targets, const std::unordered_map<TimelineTargetID, i32>& idToIndexMap)
		{
			if (targets.size() != idToIndexMap.size())
				assert(false);

			for (i32 index = 0; index < static_cast<i32>(targets.size()); index++)
			{
				const auto& target = targets[index];
				const auto foundIDToIndexEntry = idToIndexMap.find(target.ID);

				if (foundIDToIndexEntry == idToIndexMap.end())
					assert(false);

				if (foundIDToIndexEntry->second != index)
					assert(false);
			}
		}
#else
		inline void AssertTargetListOrder(const std::vector<TimelineTarget>& targets) {}
		inline void AssertTargetListFlags(const std::vector<TimelineTarget>& targets) {}
		inline void AssertTargetIDToIndexMap(const std::vector<TimelineTarget>& targets, const std::unordered_map<TimelineTargetID, i32>& idToIndexMap) {}
#endif

		inline void AssertSortedTargetListEntireState(const std::vector<TimelineTarget>& targets, const std::unordered_map<TimelineTargetID, i32>& idToIndexMap)
		{
			AssertTargetListOrder(targets);
			AssertTargetListFlags(targets);
			AssertTargetIDToIndexMap(targets, idToIndexMap);
		}
	}

	TimelineTargetID SortedTargetList::GetNextUniqueID()
	{
		return static_cast<TimelineTargetID>(++GlobalTimelineTargetIDCounter);
	}

	SortedTargetList::SortedTargetList()
	{
		constexpr auto reasonableInitialCapacity = 2000;
		targets.reserve(reasonableInitialCapacity);
	}

	TimelineTargetID SortedTargetList::Add(TimelineTarget newTarget)
	{
		if (newTarget.ID == TimelineTargetID::Null)
			newTarget.ID = GetNextUniqueID();

		const auto insertionIndex = FindSortedInsertionIndex(newTarget.Tick, newTarget.Type);
		targets.emplace(targets.begin() + insertionIndex, newTarget);

		for (i32 i = static_cast<i32>(insertionIndex); i < static_cast<i32>(targets.size()); i++)
			idToIndexMap[targets[i].ID] = i;

		UpdateTargetInternalFlagsAround(static_cast<i32>(insertionIndex));

		// AssertTargetListOrder(targets);
		// AssertTargetIDToIndexMap(targets, idToIndexMap);
		AssertSortedTargetListEntireState(targets, idToIndexMap);

		return newTarget.ID;
	}

	void SortedTargetList::Remove(TimelineTarget target)
	{
		RemoveAt(FindIndex(target.Tick, target.Type));
	}

	void SortedTargetList::RemoveAt(i32 index)
	{
		if (!InBounds(static_cast<size_t>(index), targets))
			return;

		AssertTargetIDToIndexMap(targets, idToIndexMap);

		for (i32 i = index + 1; i < static_cast<i32>(targets.size()); i++)
			idToIndexMap.find(targets[i].ID)->second--;

		idToIndexMap.erase(targets[index].ID);
		targets.erase(begin() + index);

		UpdateTargetInternalFlagsAround(index);

		AssertTargetIDToIndexMap(targets, idToIndexMap);
	}

	i32 SortedTargetList::FindIndex(BeatTick tick) const
	{
		// TODO: Binary search
		const auto foundIndex = FindIndexOf(targets, [&](const auto& target) { return (target.Tick == tick); });
		return InBounds(foundIndex, targets) ? static_cast<i32>(foundIndex) : -1;
	}

	i32 SortedTargetList::FindIndex(BeatTick tick, ButtonType type) const
	{
		// TODO: Binary search
		const auto foundIndex = FindIndexOf(targets, [&](const auto& target) { return (target.Tick == tick) && (target.Type == type); });
		return InBounds(foundIndex, targets) ? static_cast<i32>(foundIndex) : -1;
	}

	i32 SortedTargetList::FindIndex(TimelineTargetID id) const
	{
#if COMFY_DEBUG && 0
		const auto linearFoundIndex = FindIndexOf(targets, [id](auto& t) { return t.ID == id; });
		return InBounds(linearFoundIndex, targets) ? static_cast<i32>(linearFoundIndex) : -1;
#endif

		assert(id != TimelineTargetID::Null);
		AssertTargetIDToIndexMap(targets, idToIndexMap);

		const auto foundIt = idToIndexMap.find(id);
		if (foundIt == idToIndexMap.end())
			return -1;

		const auto foundIndex = foundIt->second;
		assert(InBounds(foundIndex, targets) && targets[foundIndex].ID == id);
		return foundIndex;
	}

	void SortedTargetList::Clear()
	{
		targets.clear();
		idToIndexMap.clear();
	}

	void SortedTargetList::ExplicitlyUpdateFlagsAndSortEverything()
	{
		ExplicitlyUpdateFlagsAndSortIndexRange(-1, -1);
	}

	void SortedTargetList::ExplicitlyUpdateFlagsAndSortIndexRange(i32 startIndex, i32 endIndex)
	{
		if (targets.empty())
			return;

		if (startIndex <= -1)
			startIndex = 0;

		if (endIndex <= -1 || endIndex > targets.size())
			endIndex = static_cast<i32>(targets.size());

		startIndex = FloorIndexToSyncPairStart(startIndex);
		endIndex = CeilIndexToSyncPairEnd(endIndex);

		std::sort(
			std::clamp(targets.begin() + startIndex, targets.begin(), targets.end()),
			std::clamp(targets.begin() + endIndex, targets.begin(), targets.end()),
			[&](auto& a, auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); });

		for (i32 i = startIndex; i <= endIndex; i++)
		{
			if (InBounds(i, targets))
				idToIndexMap[targets[i].ID] = i;
		}

		UpdateTargetInternalFlagsInRange(startIndex, endIndex);

		// AssertTargetListFlags(targets);
		// AssertTargetIDToIndexMap(targets, idToIndexMap);
		AssertSortedTargetListEntireState(targets, idToIndexMap);
	}

	void SortedTargetList::operator=(std::vector<TimelineTarget>&& newTargets)
	{
		targets = std::move(newTargets);

		idToIndexMap.clear();
		idToIndexMap.reserve(targets.size());

		for (i32 i = 0; i < static_cast<i32>(targets.size()); i++)
		{
			assert(targets[i].ID == TimelineTargetID::Null);
			idToIndexMap[targets[i].ID = GetNextUniqueID()] = i;
		}

		std::sort(targets.begin(), targets.end(), [&](auto& a, auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); });
		UpdateTargetInternalFlagsInRange();
	}

	size_t SortedTargetList::FindSortedInsertionIndex(BeatTick tick, ButtonType type) const
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

	void SortedTargetList::UpdateTargetInternalFlagsAround(i32 index)
	{
		constexpr auto surroundingTargetsToCheck = 1;
		UpdateTargetInternalFlagsInRange(index - surroundingTargetsToCheck, index + surroundingTargetsToCheck);
	}

	void SortedTargetList::UpdateTargetInternalFlagsInRange(i32 startIndex, i32 endIndex)
	{
		if (startIndex < 0)
			startIndex = 0;

		const auto targetCount = static_cast<i32>(targets.size());
		if (endIndex < 0 || endIndex > targetCount)
			endIndex = targetCount;

		const auto syncStartIndex = FloorIndexToSyncPairStart(startIndex);
		const auto syncEndIndex = CeilIndexToSyncPairEnd(endIndex);
		UpdateSyncPairFlagsInRange(syncStartIndex, syncEndIndex);

		// TODO: Floor and ceil to "slide -> slide chain" transitions (?)
		UpdateChainFlagsInRange(0, static_cast<i32>(targets.size()));
	}

	i32 SortedTargetList::FloorIndexToSyncPairStart(i32 index) const
	{
		i32 syncPairStartIndex = index;
		for (i32 i = (index - 1); i >= 0; i--)
		{
			if (targets[i].Tick == targets[index].Tick)
				syncPairStartIndex--;
			else
				break;
		}
		return syncPairStartIndex;
	}

	i32 SortedTargetList::CeilIndexToSyncPairEnd(i32 index) const
	{
		i32 syncPairEndIndex = index;
		for (i32 i = index; i < static_cast<i32>(targets.size()); i++)
		{
			if (targets[i].Tick == targets[index].Tick)
				syncPairEndIndex++;
			else
				break;
		}
		return syncPairEndIndex;
	}

	i32 SortedTargetList::CountConsecutiveSyncTargets(i32 startIndex) const
	{
		for (i32 i = startIndex; i < static_cast<i32>(targets.size()); i++)
		{
			const auto& target = targets[i];
			const auto* nextTarget = IndexOrNull(i + 1, targets);

			if (nextTarget == nullptr || target.Tick != nextTarget->Tick)
				return (i - startIndex) + 1;
		}

		return 1;
	}

	i32 SortedTargetList::CountConsecutiveSameTypeSyncTargets(i32 startIndex) const
	{
		for (i32 i = startIndex; i < static_cast<i32>(targets.size()); i++)
		{
			const auto& target = targets[i];
			const auto* nextTarget = IndexOrNull(i + 1, targets);

			if (nextTarget == nullptr || target.Tick != nextTarget->Tick || target.Type != nextTarget->Type)
				return (i - startIndex) + 1;
		}

		return 1;
	}

	void SortedTargetList::UpdateSyncPairFlagsInRange(i32 startIndex, i32 endIndex)
	{
		for (i32 i = startIndex; i < endIndex;)
		{
			const auto pairStartIndex = i;
			const auto pairCount = CountConsecutiveSyncTargets(i);
			assert(pairCount > 0);

			for (i32 pairIndex = 0; pairIndex < pairCount; pairIndex++)
			{
				auto& target = targets[i++];
				target.Flags.IsSync = (pairCount > 1);
				target.Flags.IndexWithinSyncPair = pairIndex;
				target.Flags.SyncPairCount = pairCount;
			}

			for (i32 pairIndex = 0; pairIndex < pairCount;)
			{
				const auto sameTypeCount = CountConsecutiveSameTypeSyncTargets(pairStartIndex + pairIndex);
				assert(sameTypeCount > 0);

				for (i32 sameTypeIndex = 0; sameTypeIndex < sameTypeCount; sameTypeIndex++)
				{
					auto& thisTarget = targets[pairStartIndex + pairIndex + sameTypeIndex];
					thisTarget.Flags.SameTypeSyncIndex = sameTypeIndex;
					thisTarget.Flags.SameTypeSyncCount = sameTypeCount;
				}
				pairIndex += sameTypeCount;
			}
		}
	}

	void SortedTargetList::UpdateChainFlagsInRange(i32 startIndex, i32 endIndex)
	{
		UpdateChainFlagsForDirection(startIndex, endIndex, ButtonType::SlideL);
		UpdateChainFlagsForDirection(startIndex, endIndex, ButtonType::SlideR);
	}

	void SortedTargetList::UpdateChainFlagsForDirection(i32 startIndex, i32 endIndex, ButtonType slideDirection)
	{
		assert(IsSlideButtonType(slideDirection));

		for (i32 i = startIndex; i < endIndex; i++)
		{
			auto& target = targets[i];
			if (target.Type == slideDirection)
			{
				target.Flags.IsChainStart = false;
				target.Flags.IsChainEnd = false;
				slideTargetBuffer.push_back(&target);
			}
		}

		if (slideTargetBuffer.empty())
			return;

		for (i32 i = 0; i < static_cast<i32>(slideTargetBuffer.size()); i++)
		{
			auto& target = *slideTargetBuffer[i];
			auto* nextTarget = (i + 1 < slideTargetBuffer.size()) ? slideTargetBuffer[i + 1] : nullptr;
			auto* prevTarget = (i - 1 >= 0) ? slideTargetBuffer[i - 1] : nullptr;

			if (prevTarget == nullptr)
			{
				if (target.Flags.IsChain)
					target.Flags.IsChainStart = true;
			}
			else
			{
				// HACK: Not sure if there exists a better solution. If the threshold is determined by time instead
				//		 then each target will need to store a cached time tick field that has to be kept in sync with the tempo map
				constexpr auto chainFragThreshold = (BeatTick::FromBars(1) / 12);
				const auto tickDistToLast = (target.Tick - prevTarget->Tick);

				if ((tickDistToLast > chainFragThreshold) || (prevTarget->Flags.IsChain != target.Flags.IsChain))
				{
					if (prevTarget->Flags.IsChain)
						prevTarget->Flags.IsChainEnd = true;

					if (target.Flags.IsChain)
						target.Flags.IsChainStart = true;
				}
			}
		}

		if (auto& back = *slideTargetBuffer.back(); back.Flags.IsChain)
			back.Flags.IsChainEnd = true;

		slideTargetBuffer.clear();
	}
}
