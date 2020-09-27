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

	void SortedTargetList::Add(TimelineTarget newTarget)
	{
#if 1 // NOTE: Sorted insertion, appears to be much faster
		{
			const auto insertionIndex = FindSortedInsertionIndex(newTarget.Tick, newTarget.Type);
			targets.emplace(targets.begin() + insertionIndex, newTarget);

			UpdateTargetInternalFlagsAround(static_cast<i32>(insertionIndex));
		}
#else // NOTE: However a post emplace sort is useful for error checking
		{
			targets.push_back(newTarget);
			std::sort(targets.begin(), targets.end(), [&](const auto& a, const auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); });

			UpdateTargetSyncFlags();
		}
#endif

		assert(std::is_sorted(targets.begin(), targets.end(), [&](const auto& a, const auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); }));
	}

	void SortedTargetList::Remove(TimelineTarget target)
	{
		RemoveAt(FindIndex(target.Tick, target.Type));
	}

	void SortedTargetList::RemoveAt(i32 index)
	{
		if (!InBounds(static_cast<size_t>(index), targets))
			return;

		targets.erase(begin() + index);
		UpdateTargetInternalFlagsAround(index);
	}

	i32 SortedTargetList::FindIndex(TimelineTick tick) const
	{
		// TODO: Binary search
		const auto foundIndex = FindIndexOf(targets, [&](const auto& target) { return (target.Tick == tick); });
		return InBounds(foundIndex, targets) ? static_cast<i32>(foundIndex) : -1;
	}

	i32 SortedTargetList::FindIndex(TimelineTick tick, ButtonType type) const
	{
		// TODO: Binary search
		const auto foundIndex = FindIndexOf(targets, [&](const auto& target) { return (target.Tick == tick) && (target.Type == type); });
		return InBounds(foundIndex, targets) ? static_cast<i32>(foundIndex) : -1;
	}

	void SortedTargetList::Clear()
	{
		targets.clear();
	}

	void SortedTargetList::operator=(std::vector<TimelineTarget>&& newTargets)
	{
		targets = std::move(newTargets);

		std::sort(targets.begin(), targets.end(), [&](const auto& a, const auto& b) { return GetTargetSortWeight(a) < GetTargetSortWeight(b); });
		UpdateTargetInternalFlagsAroundRange();
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

	void SortedTargetList::UpdateTargetInternalFlagsAround(i32 index)
	{
		constexpr auto surroundingTargetsToCheck = static_cast<i32>(EnumCount<ButtonType>());

		UpdateTargetInternalFlagsAroundRange(
			index - surroundingTargetsToCheck,
			index + surroundingTargetsToCheck);
	}

	void SortedTargetList::UpdateTargetInternalFlagsAroundRange(i32 start, i32 end)
	{
		if (start < 0)
			start = 0;

		const auto targetCount = static_cast<i32>(targets.size());

		if (end < 0 || end > targetCount)
			end = targetCount;

		for (i32 i = start; i < end; i++)
		{
			auto& target = targets[i];
			const auto* nextTarget = (i + 1 < targetCount) ? &targets[i + 1] : nullptr;
			const auto* prevTarget = (i - 1 >= 0) ? &targets[i - 1] : nullptr;

			target.Flags.IsSync = (prevTarget != nullptr && prevTarget->Tick == target.Tick) || (nextTarget != nullptr && nextTarget->Tick == target.Tick);
		}

		// TODO: Only update in the start and end index range
		UpdateSyncPairIndexFlags();
		UpdateChainFlags();
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

	void SortedTargetList::UpdateChainFlags()
	{
		UpdateChainFlagsForDirection(ButtonType::SlideL);
		UpdateChainFlagsForDirection(ButtonType::SlideR);
	}

	void SortedTargetList::UpdateChainFlagsForDirection(ButtonType slideDirection)
	{
		assert(IsSlideButtonType(slideDirection));

		for (auto& target : targets)
		{
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
				constexpr auto chainFragThreshold = (TimelineTick::FromBars(1) / 12);
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
