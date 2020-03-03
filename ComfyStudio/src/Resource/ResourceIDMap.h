#pragma once
#include "IDTypes.h"
#include "CoreTypes.h"

namespace Comfy
{
#define DEBUG_ASSERT_SORTED() assert(std::is_sorted(sortedResources.begin(), sortedResources.end(), [](const auto& a, const auto& b) { return a.ID < b.ID; }));

	// NOTE: High performance container to map between (ID -> Resource) using CachedResourceIDs
	template <typename IDType, typename ResourceType>
	class ResourceIDMap
	{
		// NOTE: Completly arbitrary threshold, might wanna do some real performance testing
		static constexpr size_t LinearSearchThreshold = 32;

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
				sortedResources[indexOrClosest].Resource = resource;
			else
				sortedResources.insert(sortedResources.begin() + indexOrClosest, { id, resource });

			DEBUG_ASSERT_SORTED();
		}

		template <typename CollectionType, typename Func>
		void AddRange(CollectionType& collection, Func func)
		{
			sortedResources.reserve(sortedResources.size() + collection.size());

			for (auto& item : collection)
				sortedResources.push_back(func(item));

			SortAllResources();
		}

		void Remove(IDType id)
		{
			RemoveResource(id);
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
			DEBUG_ASSERT_SORTED();

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
			size_t IndexOrClosest;
			bool WasFound;
		};

		FindIndexResult FindIndex(IDType id) const
		{
			return (sortedResources.size() < LinearSearchThreshold) ? FindIndexLinearSearch(id) : FindIndexBinarySearch(id);
		}

		FindIndexResult FindIndexLinearSearch(IDType id) const
		{
			for (size_t i = 0; i < sortedResources.size(); i++)
			{
				if (sortedResources[i].ID == id)
					return { i, true };
				else if (sortedResources[i].ID > id)
					return { i, false };
			}
			return { sortedResources.size(), false };
		}

		FindIndexResult FindIndexBinarySearch(IDType id) const
		{
			if (sortedResources.size() < 1)
				return { sortedResources.size(), false };

			size_t left = 0, right = sortedResources.size() - 1;
			while (left <= right)
			{
				const size_t mid = (left + right) / 2;

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

		void RemoveResource(IDType id)
		{
			sortedResources.erase(std::remove_if(sortedResources.begin(), sortedResources.end(), [id](auto& pair) { return pair.ID = id; }), sortedResources.end());
		}

		std::vector<ResourceIDPair> sortedResources;
	};
}
