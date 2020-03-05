#pragma once
#include "IDTypes.h"
#include "CoreTypes.h"

namespace Comfy
{
	// NOTE: High performance container to map between (ID -> Resource) using CachedResourceIDs
	template <typename IDType, typename ResourceType>
	class ResourceIDMap
	{
		// NOTE: Completly arbitrary threshold, might wanna do some real performance testing
		static constexpr size_t LinearSearchThreshold = 32;
		static constexpr bool OverideDuplicateIDs = true;

	public:
		struct ResourceIDPair
		{
			IDType ID;
			RefPtr<ResourceType> Resource;
		};

		bool Contains(IDType id) const
		{
			return FindIndex(id).WasFound;
		}

		void Add(IDType id, const RefPtr<ResourceType>& resource)
		{
			if (id == IDType::Invalid)
				return;

			if (const auto[indexOrClosest, wasFound] = FindIndex(id); wasFound)
			{
				if (OverideDuplicateIDs)
					sortedResources[indexOrClosest].Resource = resource;
			}
			else
			{
				sortedResources.insert(sortedResources.begin() + indexOrClosest, { id, resource });
			}
		}

		template <typename CollectionType, typename Func>
		void AddRange(CollectionType& collection, Func func)
		{
			sortedResources.reserve(sortedResources.size() + collection.size());

			for (auto& item : collection)
			{
				auto resourceToAdd = func(item);
				if (const auto[indexOrClosest, wasFound] = FindIndex(resourceToAdd.ID); wasFound)
				{
					if (OverideDuplicateIDs)
						sortedResources[indexOrClosest].Resource = std::move(resourceToAdd.Resource);
				}
				else
				{
					sortedResources.push_back(std::move(resourceToAdd));
				}
			}

			SortAllResources();
		}

		void Remove(IDType id)
		{
			sortedResources.erase(
				std::remove_if(
					sortedResources.begin(),
					sortedResources.end(),
					[id](auto& pair) { return pair.ID == id; }),
				sortedResources.end());
		}

		template <typename Func>
		void RemoveRange(Func func)
		{
			sortedResources.erase(
				std::remove_if(
					sortedResources.begin(),
					sortedResources.end(),
					func),
				sortedResources.end());
		}

		void Clear()
		{
			sortedResources.clear();
		}

		size_t Size() const
		{
			return sortedResources.size();
		}

		ResourceType* Find(const IDType& rawID) const = delete;

		ResourceType* Find(const CachedResourceID<IDType>& cachedID) const
		{
			if (cachedID == IDType::Invalid)
				return nullptr;

			if (cachedID.CachedIndex < sortedResources.size())
			{
				if (sortedResources[cachedID.CachedIndex].ID == cachedID)
					return sortedResources[cachedID.CachedIndex].Resource.get();
				else
					cachedID.CachedIndex = std::numeric_limits<decltype(cachedID.CachedIndex)>::max();
			}

			if (const auto[index, wasFound] = FindIndex(cachedID.ID); wasFound)
			{
				cachedID.CachedIndex = static_cast<decltype(cachedID.CachedIndex)>(index);
				return sortedResources[index].Resource.get();
			}

			return nullptr;
		}

	private:
		struct FindIndexResult
		{
			int IndexOrClosest;
			bool WasFound;
		};

		FindIndexResult FindIndex(IDType id) const
		{
			return (sortedResources.size() < LinearSearchThreshold) ? FindIndexLinearSearch(id) : FindIndexBinarySearch(id);
		}

		FindIndexResult FindIndexLinearSearch(IDType id) const
		{
			const int resourceCount = static_cast<int>(sortedResources.size());
			for (int i = 0; i < resourceCount; i++)
			{
				if (sortedResources[i].ID == id)
					return { i, true };
				else if (sortedResources[i].ID > id)
					return { i, false };
			}
			return { resourceCount, false };
		}

		FindIndexResult FindIndexBinarySearch(IDType id) const
		{
			if (sortedResources.size() < 1)
				return { 0, false };

			int left = 0, right = static_cast<int>(sortedResources.size()) - 1;
			while (left <= right)
			{
				const int mid = (left + right) / 2;

				if (id < sortedResources[mid].ID)
					right = (mid - 1);
				else if (id > sortedResources[mid].ID)
					left = (mid + 1);
				else
					return { mid, true };
			}

			return { left, false };
		}

		void SortAllResources()
		{
			std::sort(sortedResources.begin(), sortedResources.end(), [](auto& a, auto& b) { return a.ID < b.ID; });
		}

		std::vector<ResourceIDPair> sortedResources;
	};
}
