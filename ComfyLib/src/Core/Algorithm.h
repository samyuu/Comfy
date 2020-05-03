#pragma once
#include "CoreMacros.h"
#include <type_traits>

namespace Comfy
{
	template <typename IndexType, typename ArrayType>
	COMFY_NODISCARD constexpr __forceinline auto InBounds(const IndexType& index, const ArrayType& array) -> bool
	{
		static_assert(std::is_integral<IndexType>::value);

		if constexpr (std::is_signed<IndexType>::value)
			return (index >= 0 && index < array.size());
		else
			return (index < array.size());
	}

	template <typename CollectionType, typename Func>
	COMFY_NODISCARD auto FindIfOrNull(CollectionType& collection, Func predicate) -> typename CollectionType::value_type*
	{
		for (auto it = std::begin(collection); it != std::end(collection); it++)
		{
			if (predicate(*it))
				return &(*it);
		}
		return nullptr;
	}

	template <typename CollectionType, typename Func>
	COMFY_NODISCARD auto FindIfOrNull(const CollectionType& collection, Func predicate) -> const typename CollectionType::value_type*
	{
		for (auto it = std::begin(collection); it != std::end(collection); it++)
		{
			if (predicate(*it))
				return &(*it);
		}
		return nullptr;
	}
}
