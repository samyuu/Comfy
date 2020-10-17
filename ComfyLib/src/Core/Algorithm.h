#pragma once
#include "CoreMacros.h"
#include <type_traits>

namespace Comfy
{
	// NOTE: Assumes the enum class EnumType { ..., Count }; convention to be used everywhere
	template <typename EnumType>
	COMFY_NODISCARD constexpr size_t EnumCount()
	{
		static_assert(std::is_enum_v<EnumType>);
		return static_cast<size_t>(EnumType::Count);
	}

	template <typename IndexType, typename ArrayType>
	COMFY_NODISCARD constexpr __forceinline auto InBounds(const IndexType& index, const ArrayType& array) -> bool
	{
		static_assert(std::is_integral_v<IndexType>);
		using SizeType = decltype(array.size());

		if constexpr (std::is_signed_v<IndexType>)
			return (index >= 0 && static_cast<SizeType>(index) < array.size());
		else
			return (index < array.size());
	}

	template <typename IndexType, typename ArrayType, typename DefaultType>
	COMFY_NODISCARD constexpr __forceinline auto IndexOr(const IndexType& index, ArrayType& array, DefaultType defaultValue) -> auto&
	{
		return InBounds(index, array) ? array[index] : defaultValue;
	}

	template <typename IndexType, typename ArrayType>
	COMFY_NODISCARD constexpr __forceinline auto IndexOrNull(const IndexType& index, ArrayType& array) -> auto*
	{
		return InBounds(index, array) ? &array[index] : nullptr;
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

	template <typename CollectionType, typename Func>
	COMFY_NODISCARD auto FindIndexOf(CollectionType& collection, Func predicate) -> size_t
	{
		size_t index = 0;
		for (auto it = std::begin(collection); it != std::end(collection); it++)
		{
			if (predicate(*it))
				return index;
			index++;
		}
		return index;
	}

	template <typename CollectionType, typename Func>
	COMFY_NODISCARD auto FindLastIndexOf(CollectionType& collection, Func predicate) -> size_t
	{
		size_t index = std::size(collection) - 1;
		for (auto it = std::rbegin(collection); it != std::rend(collection); it++)
		{
			if (predicate(*it))
				return index;
			index--;
		}
		return index;
	}

	template <typename FloatType>
	COMFY_NODISCARD constexpr auto ConvertRange(FloatType originalStart, FloatType originalEnd, FloatType newStart, FloatType newEnd, FloatType value) -> FloatType
	{
		static_assert(std::is_floating_point_v<FloatType>);
		return (newStart + ((value - originalStart) * (newEnd - newStart) / (originalEnd - originalStart)));
	}
}
