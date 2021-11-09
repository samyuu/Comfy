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

using ivec2 = glm::vec<2, i32, glm::defaultp>;
using ivec3 = glm::vec<3, i32, glm::defaultp>;
using ivec4 = glm::vec<4, i32, glm::defaultp>;

using uvec2 = glm::vec<2, u32, glm::defaultp>;
using uvec3 = glm::vec<3, u32, glm::defaultp>;
using uvec4 = glm::vec<4, u32, glm::defaultp>;

using vec2 = glm::vec<2, f32, glm::defaultp>;
using vec3 = glm::vec<3, f32, glm::defaultp>;
using vec4 = glm::vec<4, f32, glm::defaultp>;

using dvec2 = glm::vec<2, f64, glm::defaultp>;
using dvec3 = glm::vec<3, f64, glm::defaultp>;
using dvec4 = glm::vec<4, f64, glm::defaultp>;

using mat3 = glm::mat<3, 3, f32, glm::defaultp>;
using mat4 = glm::mat<4, 4, f32, glm::defaultp>;

using quat = glm::qua<f32, glm::defaultp>;

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

#if 0 // TODO:
constexpr f32 PI = 3.14159265358979323846f;
constexpr f32 DegreesToRadians = 0.01745329251994329577f;
constexpr f32 RadiansToDegrees = 57.2957795130823208768f;

struct Angle
{
	f32 Radians;

	constexpr f32 ToRadians() const { return Radians; }
	constexpr f32 ToDegrees() const { return Radians * RadiansToDegrees; }
	static constexpr Angle FromRadians(f32 radians) { return Angle { radians }; }
	static constexpr Angle FromDegrees(f32 degrees) { return Angle { degrees * DegreesToRadians }; }
};

static_assert(sizeof(Angle) == sizeof(f32) && std::is_trivial_v<Angle>);
#endif

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
	COMFY_NODISCARD constexpr __forceinline bool InBounds(const IndexType& index, const ArrayType& array)
	{
		static_assert(std::is_integral_v<IndexType>);
		using SizeType = decltype(array.size());

		if constexpr (std::is_signed_v<IndexType>)
			return (index >= 0 && static_cast<SizeType>(index) < array.size());
		else
			return (index < array.size());
	}

	template <typename IndexType, typename ArrayType, typename DefaultType>
	COMFY_NODISCARD constexpr __forceinline auto IndexOr(const IndexType& index, ArrayType& array, DefaultType defaultValue)
	{
		return InBounds(index, array) ? array[index] : defaultValue;
	}

	template <typename IndexType, typename ArrayType>
	COMFY_NODISCARD constexpr __forceinline auto* IndexOrNull(const IndexType& index, ArrayType& array)
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
	COMFY_NODISCARD size_t FindIndexOf(CollectionType& collection, Func predicate)
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
	COMFY_NODISCARD size_t FindLastIndexOf(CollectionType& collection, Func predicate)
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

	COMFY_NODISCARD constexpr f32 ToPercent(f32 value) { return (value * 100.0f); }
	COMFY_NODISCARD constexpr vec2 ToPercent(vec2 value) { return { ToPercent(value.x), ToPercent(value.y) }; }

	COMFY_NODISCARD constexpr f32 FromPercent(f32 percent) { return (percent * 0.01f); }
	COMFY_NODISCARD constexpr vec2 FromPercent(vec2 percent) { return { FromPercent(percent.x), FromPercent(percent.y) }; }

	template <typename T> COMFY_NODISCARD constexpr T Min(T x, T y) { return (y < x) ? y : x; }
	template <> COMFY_NODISCARD constexpr ivec2 Min<ivec2>(ivec2 x, ivec2 y) { return { Min(x.x, y.x), Min(x.y, y.y) }; }
	template <> COMFY_NODISCARD constexpr ivec3 Min<ivec3>(ivec3 x, ivec3 y) { return { Min(x.x, y.x), Min(x.y, y.y), Min(x.z, y.z) }; }
	template <> COMFY_NODISCARD constexpr ivec4 Min<ivec4>(ivec4 x, ivec4 y) { return { Min(x.x, y.x), Min(x.y, y.y), Min(x.z, y.z), Min(x.w, y.w) }; }
	template <> COMFY_NODISCARD constexpr vec2 Min<vec2>(vec2 x, vec2 y) { return { Min(x.x, y.x), Min(x.y, y.y) }; }
	template <> COMFY_NODISCARD constexpr vec3 Min<vec3>(vec3 x, vec3 y) { return { Min(x.x, y.x), Min(x.y, y.y), Min(x.z, y.z) }; }
	template <> COMFY_NODISCARD constexpr vec4 Min<vec4>(vec4 x, vec4 y) { return { Min(x.x, y.x), Min(x.y, y.y), Min(x.z, y.z), Min(x.w, y.w) }; }

	template <typename T> COMFY_NODISCARD constexpr T Max(T x, T y) { return (x < y) ? y : x; }
	template <> COMFY_NODISCARD constexpr ivec2 Max<ivec2>(ivec2 x, ivec2 y) { return { Max(x.x, y.x), Max(x.y, y.y) }; }
	template <> COMFY_NODISCARD constexpr ivec3 Max<ivec3>(ivec3 x, ivec3 y) { return { Max(x.x, y.x), Max(x.y, y.y), Max(x.z, y.z) }; }
	template <> COMFY_NODISCARD constexpr ivec4 Max<ivec4>(ivec4 x, ivec4 y) { return { Max(x.x, y.x), Max(x.y, y.y), Max(x.z, y.z), Max(x.w, y.w) }; }
	template <> COMFY_NODISCARD constexpr vec2 Max<vec2>(vec2 x, vec2 y) { return { Max(x.x, y.x), Max(x.y, y.y) }; }
	template <> COMFY_NODISCARD constexpr vec3 Max<vec3>(vec3 x, vec3 y) { return { Max(x.x, y.x), Max(x.y, y.y), Max(x.z, y.z) }; }
	template <> COMFY_NODISCARD constexpr vec4 Max<vec4>(vec4 x, vec4 y) { return { Max(x.x, y.x), Max(x.y, y.y), Max(x.z, y.z), Max(x.w, y.w) }; }

	template <typename T> COMFY_NODISCARD constexpr T Clamp(T value, T min, T max) { return Min<T>(Max<T>(value, min), max); }

	template <typename T>
	COMFY_NODISCARD constexpr T ConvertRange(T originalStart, T originalEnd, T newStart, T newEnd, T value)
	{
		static_assert(std::is_floating_point_v<T>);
		return (newStart + ((value - originalStart) * (newEnd - newStart) / (originalEnd - originalStart)));
	}

	template <typename T>
	COMFY_NODISCARD constexpr T ConvertRangeClamped(T originalStart, T originalEnd, T newStart, T newEnd, T value)
	{
		return Clamp<T>(ConvertRange<T>(originalStart, originalEnd, newStart, newEnd, value), newStart, newEnd);
	}
}
