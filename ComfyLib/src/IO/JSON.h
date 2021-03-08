#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include <optional>
#include <future>

#define JSON_NOEXCEPTION
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#define JSON_COMFY_FORMATTING
#include <nlohmann/json.hpp>

namespace Comfy
{
	using json = nlohmann::json;

	// NOTE: Naming the "JSON" symbols uppercase might be "more correct" but ends up looking worse and less readable than pascal case "Json"
	//		 and is also how the .NET JSON library names it

	constexpr std::string_view JsonExtension = ".json";
	constexpr std::string_view JsonFilterName = "JSON (*.json)";
	constexpr std::string_view JsonFilterSpec = "*.json";

	namespace JsonDetail
	{
		constexpr bool AllowExceptions = false;
		constexpr bool IgnoreComments = true;
		constexpr i32 Indentation = 4;
		constexpr char IndentationChar = ' ';
		constexpr bool EnsureASCII = false;
		constexpr json::error_handler_t ErrorHandler = json::error_handler_t::ignore;

		template <typename Type, json::value_t TypeEnum>
		std::optional<Type> TryGet(const json& j) { if (j.type() == TypeEnum) { return j.get<Type>(); } else { return std::nullopt; } }

		template <typename VecType>
		std::optional<VecType> TryGetVec(const json& j)
		{
			const std::array<std::string, 4> componentNames = { "x", "y", "z", "w" };
			VecType result;

			for (typename VecType::length_type i = 0; i < VecType::length(); i++)
			{
				if (auto foundComponent = j.find(componentNames[i]); foundComponent != j.end())
				{
					const json& component = *foundComponent;
					if (std::is_floating_point_v<typename VecType::value_type> ? component.is_number_float() : component.is_number_integer())
						result[i] = component.get<typename VecType::value_type>();
					else
						return std::nullopt;
				}
				else
				{
					return std::nullopt;
				}
			}

			return result;
		}
	}

	inline auto JsonTryGetBool(const json& j) { return JsonDetail::TryGet<bool, json::value_t::boolean>(j); }
	inline auto JsonTryGetBool(const json* j) { return (j != nullptr) ? JsonTryGetBool(*j) : std::nullopt; }
	inline auto JsonTryGetI32(const json& j) { return JsonDetail::TryGet<i32, json::value_t::number_integer>(j); }
	inline auto JsonTryGetI32(const json* j) { return (j != nullptr) ? JsonTryGetI32(*j) : std::nullopt; }
	inline auto JsonTryGetU32(const json& j) { return JsonDetail::TryGet<u32, json::value_t::number_unsigned>(j); }
	inline auto JsonTryGetU32(const json* j) { return (j != nullptr) ? JsonTryGetU32(*j) : std::nullopt; }
	inline auto JsonTryGetI64(const json& j) { return JsonDetail::TryGet<i64, json::value_t::number_integer>(j); }
	inline auto JsonTryGetI64(const json* j) { return (j != nullptr) ? JsonTryGetI64(*j) : std::nullopt; }
	inline auto JsonTryGetU64(const json& j) { return JsonDetail::TryGet<u64, json::value_t::number_unsigned>(j); }
	inline auto JsonTryGetU64(const json* j) { return (j != nullptr) ? JsonTryGetU64(*j) : std::nullopt; }
	inline auto JsonTryGetF32(const json& j) { return JsonDetail::TryGet<f32, json::value_t::number_float>(j); }
	inline auto JsonTryGetF32(const json* j) { return (j != nullptr) ? JsonTryGetF32(*j) : std::nullopt; }
	inline auto JsonTryGetF64(const json& j) { return JsonDetail::TryGet<f64, json::value_t::number_float>(j); }
	inline auto JsonTryGetF64(const json* j) { return (j != nullptr) ? JsonTryGetF64(*j) : std::nullopt; }
	inline auto JsonTryGetStr(const json& j) { return JsonDetail::TryGet<std::string, json::value_t::string>(j); }
	inline auto JsonTryGetStr(const json* j) { return (j != nullptr) ? JsonTryGetStr(*j) : std::nullopt; }
	inline auto JsonTryGetIVec2(const json& j) { return JsonDetail::TryGetVec<ivec2>(j); }
	inline auto JsonTryGetIVec2(const json* j) { return (j != nullptr) ? JsonTryGetIVec2(*j) : std::nullopt; }
	inline auto JsonTryGetIVec3(const json& j) { return JsonDetail::TryGetVec<ivec3>(j); }
	inline auto JsonTryGetIVec3(const json* j) { return (j != nullptr) ? JsonTryGetIVec3(*j) : std::nullopt; }
	inline auto JsonTryGetIVec4(const json& j) { return JsonDetail::TryGetVec<ivec4>(j); }
	inline auto JsonTryGetIVec4(const json* j) { return (j != nullptr) ? JsonTryGetIVec4(*j) : std::nullopt; }
	inline auto JsonTryGetVec2(const json& j) { return JsonDetail::TryGetVec<vec2>(j); }
	inline auto JsonTryGetVec2(const json* j) { return (j != nullptr) ? JsonTryGetVec2(*j) : std::nullopt; }
	inline auto JsonTryGetVec3(const json& j) { return JsonDetail::TryGetVec<vec3>(j); }
	inline auto JsonTryGetVec3(const json* j) { return (j != nullptr) ? JsonTryGetVec3(*j) : std::nullopt; }
	inline auto JsonTryGetVec4(const json& j) { return JsonDetail::TryGetVec<vec4>(j); }
	inline auto JsonTryGetVec4(const json* j) { return (j != nullptr) ? JsonTryGetVec4(*j) : std::nullopt; }

	inline void JsonSetIVec2(json& j, const ivec2& v) { j["x"] = v.x; j["y"] = v.y; }
	inline void JsonSetIVec3(json& j, const ivec3& v) { j["x"] = v.x; j["y"] = v.y; j["z"] = v.z; }
	inline void JsonSetIVec4(json& j, const ivec4& v) { j["x"] = v.x; j["y"] = v.y; j["z"] = v.z; j["w"] = v.w; }
	inline void JsonSetVec2(json& j, const vec2& v) { j["x"] = v.x; j["y"] = v.y; }
	inline void JsonSetVec3(json& j, const vec3& v) { j["x"] = v.x; j["y"] = v.y; j["z"] = v.z; }
	inline void JsonSetVec4(json& j, const vec4& v) { j["x"] = v.x; j["y"] = v.y; j["z"] = v.z; j["w"] = v.w; }

	inline void JsonTrySetBool(json& j, const std::optional<bool>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetI32(json& j, const std::optional<i32>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetU32(json& j, const std::optional<u32>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetI64(json& j, const std::optional<i64>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetU64(json& j, const std::optional<u64>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetF32(json& j, const std::optional<f32>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetF64(json& j, const std::optional<f64>& v) { if (v.has_value()) { j = v.value(); } }
	inline void JsonTrySetIVec2(json& j, const std::optional<ivec2>& v) { if (v.has_value()) { JsonSetIVec2(j, v.value()); } }
	inline void JsonTrySetIVec3(json& j, const std::optional<ivec3>& v) { if (v.has_value()) { JsonSetIVec3(j, v.value()); } }
	inline void JsonTrySetIVec4(json& j, const std::optional<ivec4>& v) { if (v.has_value()) { JsonSetIVec4(j, v.value()); } }
	inline void JsonTrySetVec2(json& j, const std::optional<vec2>& v) { if (v.has_value()) { JsonSetVec2(j, v.value()); } }
	inline void JsonTrySetVec3(json& j, const std::optional<vec3>& v) { if (v.has_value()) { JsonSetVec3(j, v.value()); } }
	inline void JsonTrySetVec4(json& j, const std::optional<vec4>& v) { if (v.has_value()) { JsonSetVec4(j, v.value()); } }
	inline void JsonTrySetStr(json& j, const std::optional<std::string>& v) { if (v.has_value()) { j = v.value(); } }

	template <typename KeyType>
	inline json* JsonFind(json& j, KeyType&& key) { auto f = j.find(std::forward<KeyType>(key)); return (f != j.end()) ? &(*f) : nullptr; }

	template <typename KeyType>
	inline const json* JsonFind(const json& j, KeyType&& key) { auto f = j.find(std::forward<KeyType>(key)); return (f != j.cend()) ? &(*f) : nullptr; }

	COMFY_NODISCARD std::string JsonToString(const json& json);
	COMFY_NODISCARD json JsonFromString(std::string_view jsonString);
}

namespace Comfy::IO
{
	COMFY_NODISCARD std::optional<json> LoadJson(std::string_view filePath);
	COMFY_NODISCARD std::future<std::optional<json>> LoadJsonAsync(std::string_view filePath);

	/* COMFY_NODISCARD */ bool SaveJson(std::string_view filePath, const json& json);
	COMFY_NODISCARD std::future<bool> SaveJsonAsync(std::string_view filePath, const json* json);
}
