#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
#include <optional>
#include <future>

#define JSON_NOEXCEPTION
#define JSON_USE_IMPLICIT_CONVERSIONS 0
#include <nlohmann/json.hpp>

namespace Comfy
{
	using json = nlohmann::json;

	constexpr std::string_view JSONExtension = ".json";
	constexpr std::string_view JSONFilterName = "JSON (*.json)";
	constexpr std::string_view JSONFilterSpec = "*.json";

	namespace JSONDetail
	{
		constexpr bool AllowExceptions = false;
		constexpr bool IgnoreComments = true;
		constexpr i32 Indentation = 4;
		constexpr char IndentationChar = ' ';
		constexpr bool EnsureASCII = false;
		constexpr json::error_handler_t ErrorHandler = json::error_handler_t::ignore;

		template <typename Type, json::value_t TypeEnum>
		std::optional<Type> TryGet(const json& json) { if (json.type() == TypeEnum) { return json.get<Type>(); } else { return std::nullopt; } }
	}

	inline auto JSONTryGetBool(const json& json) { return JSONDetail::TryGet<bool, json::value_t::boolean>(json); }
	inline auto JSONTryGetI32(const json& json) { return JSONDetail::TryGet<i32, json::value_t::number_integer>(json); }
	inline auto JSONTryGetU32(const json& json) { return JSONDetail::TryGet<u32, json::value_t::number_unsigned>(json); }
	inline auto JSONTryGetI64(const json& json) { return JSONDetail::TryGet<i64, json::value_t::number_integer>(json); }
	inline auto JSONTryGetU64(const json& json) { return JSONDetail::TryGet<u64, json::value_t::number_unsigned>(json); }
	inline auto JSONTryGetF32(const json& json) { return JSONDetail::TryGet<f32, json::value_t::number_float>(json); }
	inline auto JSONTryGetF64(const json& json) { return JSONDetail::TryGet<f64, json::value_t::number_float>(json); }
	inline auto JSONTryGetStr(const json& json) { return JSONDetail::TryGet<std::string, json::value_t::string>(json); }

	COMFY_NODISCARD std::string JSONToString(const json& json);
	COMFY_NODISCARD json JSONFromString(std::string_view jsonString);
}

namespace Comfy::IO
{
	COMFY_NODISCARD std::optional<json> LoadJSON(std::string_view filePath);
	COMFY_NODISCARD std::future<std::optional<json>> LoadJSONAsync(std::string_view filePath);

	/* COMFY_NODISCARD */ bool SaveJSON(std::string_view filePath, const json& json);
	COMFY_NODISCARD std::future<bool> SaveJSONAsync(std::string_view filePath, const json* json);
}
