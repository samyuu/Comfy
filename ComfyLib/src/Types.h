#pragma once
#include <stdint.h>

// NOTE: Dummy types for storing still unknown data, not to be used for normal code
using unk8_t = uint8_t;
using unk16_t = uint16_t;
using unk32_t = uint32_t;
using unk64_t = uint64_t;

using i8 = int8_t;
using u8 = uint8_t;

using i16 = int16_t;
using u16 = uint16_t;

using i32 = int32_t;
using u32 = uint32_t;

using i64 = int64_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

// NOTE: Measure a frame unit / frame rate
using frame_t = float;

#include <assert.h>
#include <type_traits>
#include <memory>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <functional>

#include <glm/gtc/type_ptr.hpp>

using bvec2 = glm::vec<2, bool, glm::defaultp>;
using bvec3 = glm::vec<3, bool, glm::defaultp>;
using bvec4 = glm::vec<4, bool, glm::defaultp>;

using ivec2 = glm::vec<2, int, glm::defaultp>;
using ivec3 = glm::vec<3, int, glm::defaultp>;
using ivec4 = glm::vec<4, int, glm::defaultp>;

using uvec2 = glm::vec<2, unsigned int, glm::defaultp>;
using uvec3 = glm::vec<3, unsigned int, glm::defaultp>;
using uvec4 = glm::vec<4, unsigned int, glm::defaultp>;

using vec2 = glm::vec<2, float, glm::defaultp>;
using vec3 = glm::vec<3, float, glm::defaultp>;
using vec4 = glm::vec<4, float, glm::defaultp>;

using dvec2 = glm::vec<2, double, glm::defaultp>;
using dvec3 = glm::vec<3, double, glm::defaultp>;
using dvec4 = glm::vec<4, double, glm::defaultp>;

using mat3 = glm::mat<3, 3, float, glm::defaultp>;
using mat4 = glm::mat<4, 4, float, glm::defaultp>;

using quat = glm::qua<float, glm::defaultp>;

// NOTE: Any address, offset or pointer in file space
enum class FileAddr : i64 { NullPtr = 0 };

constexpr FileAddr operator+(FileAddr left, FileAddr right) { using T = std::underlying_type<FileAddr>::type; return static_cast<FileAddr>(static_cast<T>(left) + static_cast<T>(right)); }
constexpr FileAddr operator-(FileAddr left, FileAddr right) { using T = std::underlying_type<FileAddr>::type; return static_cast<FileAddr>(static_cast<T>(left) - static_cast<T>(right)); }
constexpr FileAddr& operator+=(FileAddr& left, FileAddr right) { using T = std::underlying_type<FileAddr>::type; return (left = static_cast<FileAddr>(static_cast<T>(left) + static_cast<T>(right))); }
constexpr FileAddr& operator-=(FileAddr& left, FileAddr right) { using T = std::underlying_type<FileAddr>::type; return (left = static_cast<FileAddr>(static_cast<T>(left) - static_cast<T>(right))); }

// NOTE: Only to be used for temporary POD* function arguments
template<typename T>
struct PtrArg
{
	T Value;
	inline explicit PtrArg(T v) : Value(v) {}

	inline constexpr operator T*() { return &Value; }
	inline constexpr operator const T*() const { return &Value; }
};

struct NonCopyable
{
	NonCopyable() = default;
	~NonCopyable() = default;

	NonCopyable(const NonCopyable&) = delete;
	NonCopyable& operator=(const NonCopyable&) = delete;
};

#if defined(COMFY_DEBUG)
#define COMFY_DEBUG_ONLY(expression) expression
#define COMFY_RELEASE_ONLY(expression)
#define COMFY_DEBUG_RELEASE_SWITCH(debug, release) (debug)
#elif defined(COMFY_RELEASE)
#define COMFY_DEBUG_ONLY(expression)
#define COMFY_RELEASE_ONLY(expression) expression
#define COMFY_DEBUG_RELEASE_SWITCH(debug, release) (release)
#endif /* COMFY_DEBUG / COMFY_RELEASE */

#define COMFY_NODISCARD [[nodiscard]]
#define COMFY_FALLTHROUGH [[fallthrough]]

#define COMFY_CONCAT_DETAIL(x,y) x##y
#define COMFY_CONCAT(x,y) COMFY_CONCAT_DETAIL(x,y)

#define COMFY_STRINGIFY_DETAIL(value) #value
#define COMFY_STRINGIFY(value) COMFY_STRINGIFY_DETAIL(value)

#define COMFY_UNIQUENAME(prefix) COMFY_CONCAT(prefix, __COUNTER__)

// NOTE: Example: defer { DoCleanup(); };
#ifndef defer
struct defer_dummy {};
template <class F> struct deferrer { F f; ~deferrer() { f(); } };
template <class F> deferrer<F> operator*(defer_dummy, F f) { return { f }; }
#define DEFER_DETAIL(LINE) zz_defer##LINE
#define DEFER(LINE) DEFER_DETAIL(LINE)
#define defer auto DEFER(__LINE__) = ::defer_dummy {} *[&]()
#endif /* defer */

// NOTE: Common helper algorithms
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
