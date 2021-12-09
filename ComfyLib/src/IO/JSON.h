#pragma once
#include "Types.h"
#include "Misc/StringUtil.h"
#include <optional>
#include <future>

#define RAPIDJSON_HAS_STDSTRING 0
#define RAPIDJSON_48BITPOINTER_OPTIMIZATION 1
#define RAPIDJSON_SSE2
#define RAPIDJSON_ASSERT(x) assert(x)
#define RAPIDJSON_PARSE_DEFAULT_FLAGS (kParseCommentsFlag | kParseTrailingCommasFlag | kParseNanAndInfFlag)
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stream.h>

// NOTE: Naming the "JSON" symbols uppercase might be "more correct" but ends up looking worse and less readable than pascal case "Json"
//		 and also matches how the .NET JSON library names it
namespace Comfy::Json
{
	constexpr std::string_view Extension = ".json";
	constexpr std::string_view FilterName = "JSON (*.json)";
	constexpr std::string_view FilterSpec = "*.json";

	constexpr i32 IndentationCharCount = 1;
	constexpr char IndentationChar = '\t';

	using Document = rapidjson::Document;
	using Allocator = rapidjson::Document::AllocatorType;
	using Value = rapidjson::Value;
	using ValueIt = rapidjson::Value::MemberIterator;
	using ConstValueIt = rapidjson::Value::ConstMemberIterator;
	using SizeType = rapidjson::SizeType;

	inline rapidjson::GenericStringRef<char> StringViewRef(std::string_view stringView) { return rapidjson::StringRef(stringView.data(), stringView.size()); }

	inline Value* Find(Document& j, std::string_view key) { ValueIt f = j.FindMember(StringViewRef(key)); return (f != j.MemberEnd()) ? &f->value : nullptr; }
	inline const Value* Find(const Document& j, std::string_view key) { ConstValueIt f = j.FindMember(StringViewRef(key)); return (f != j.MemberEnd()) ? &f->value : nullptr; }

	inline Value* Find(Value& j, std::string_view key) { ValueIt f = j.FindMember(StringViewRef(key)); return (f != j.MemberEnd()) ? &f->value : nullptr; }
	inline const Value* Find(const Value& j, std::string_view key) { ConstValueIt f = j.FindMember(StringViewRef(key)); return (f != j.MemberEnd()) ? &f->value : nullptr; }

	// NOTE: Having non null checked reference overloads would be better when iterating over arrays but I deem it not worth the extra code complecity for now

	// TODO: Make sure this accepts ints as well
	inline std::optional<bool> TryGetBool(const Value* j) { if (j != nullptr && j->IsBool()) { return j->GetBool(); } else { return std::nullopt; } }

	// TODO: Make sure this accepts floats as well
	inline std::optional<i32> TryGetI32(const Value* j) { if (j != nullptr && j->IsInt()) { return j->GetInt(); } else { return std::nullopt; } }
	inline std::optional<u32> TryGetU32(const Value* j) { if (j != nullptr && j->IsUint()) { return j->GetUint(); } else { return std::nullopt; } }

	// TODO: Make sure this accepts ints as well
	inline std::optional<f32> TryGetF32(const Value* j) { if (j != nullptr && j->IsNumber()) { return j->GetFloat(); } else { return std::nullopt; } }
	inline std::optional<f64> TryGetF64(const Value* j) { if (j != nullptr && j->IsNumber()) { return j->GetDouble(); } else { return std::nullopt; } }

	inline std::optional<std::string_view> TryGetStrView(const Value* j) { if (j != nullptr && j->IsString()) { return std::string_view(j->GetString(), j->GetStringLength()); } else { return std::nullopt; } }

	// NOTE: For both XYZW and RGBA
	inline std::optional<vec4> TryGetVec4(const Value* j)
	{
		if (j != nullptr)
		{
			auto xyzw = std::array { TryGetF32(Find(*j, "x")), TryGetF32(Find(*j, "y")), TryGetF32(Find(*j, "z")), TryGetF32(Find(*j, "w")), };
			auto rgba = std::array { TryGetF32(Find(*j, "r")), TryGetF32(Find(*j, "g")), TryGetF32(Find(*j, "b")), TryGetF32(Find(*j, "a")), };
			return vec4 { xyzw[0].value_or(rgba[0].value_or(0.0f)), xyzw[1].value_or(rgba[1].value_or(0.0f)), xyzw[2].value_or(rgba[2].value_or(0.0f)), xyzw[3].value_or(rgba[3].value_or(0.0f)), };
		}
		else
		{
			return std::nullopt;
		}
	}

	inline std::optional<u32> TryGetU32HexRGBAStr(const Value* j)
	{
		if (j != nullptr && j->IsString() && j->GetStringLength() > 1)
		{
			const char* str = j->GetString();
			if (*str == '#')
				str++;

			u32 r = 0x00, g = 0x00, b = 0x00, a = 0xFF;
			sscanf_s(str, "%02X%02X%02X%02X", &r, &g, &b, &a);

			return (a << 24) | (b << 16) | (g << 8) | (r << 0);
		}
		else
		{
			return std::nullopt;
		}
	}

	inline std::optional<vec4> TryGetVec4HexRGBAStr(const Value* j)
	{
		if (auto v = TryGetU32HexRGBAStr(j); v.has_value())
			return vec4(((*v >> 0) & 0xFF) / 255.0f, ((*v >> 8) & 0xFF) / 255.0f, ((*v >> 16) & 0xFF) / 255.0f, ((*v >> 24) & 0xFF) / 255.0f);
		else
			return std::nullopt;
	}

	template <typename Enum>
	struct EnumNameMapping
	{
		Enum EnumIndex;
		std::string_view EnumName;
	};

	template <typename Enum>
	using EnumNameMappingTable = std::array<EnumNameMapping<Enum>, EnumCount<Enum>()>;

	template <typename Enum>
	constexpr bool CompileTimeValidateEnumNameMappingTable(const EnumNameMappingTable<Enum>& nameTable)
	{
		for (size_t i = 0; i < nameTable.size(); i++)
		{
			if (nameTable[i].EnumIndex != static_cast<Enum>(i))
				return false;

			if (Util::Trim(nameTable[i].EnumName).size() != nameTable[i].EnumName.size())
				return false;
		}
		return true;
	}

	template <typename Enum>
	constexpr std::string_view EnumToStr(const EnumNameMappingTable<Enum>& nameTable, Enum enumIndex, std::string_view fallback = "")
	{
		const auto index = static_cast<size_t>(enumIndex);
		return (index < nameTable.size()) ? nameTable[index].EnumName : fallback;
	}

	template <typename Enum>
	constexpr std::optional<Enum> EnumFromStr(const EnumNameMappingTable<Enum>& nameTable, std::string_view enumString)
	{
		for (size_t i = 0; i < nameTable.size(); i++)
		{
			if (!nameTable[i].EnumName.empty() && Util::MatchesInsensitive(nameTable[i].EnumName, enumString))
				return static_cast<Enum>(i);
		}
		return std::nullopt;
	}

	template <typename Enum>
	inline std::optional<Enum> TryGetEnumStr(const Value* j, const EnumNameMappingTable<Enum>& nameTable)
	{
		if (j != nullptr && j->IsString())
			return EnumFromStr(nameTable, Util::Trim(std::string_view(j->GetString(), j->GetStringLength())));
		else
			return std::nullopt;
	}

	inline void TryAssign(bool& out, std::optional<bool>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(i32& out, std::optional<i32>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(u32& out, std::optional<u32>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(f32& out, std::optional<f32>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(f64& out, std::optional<f64>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(std::string& out, std::optional<std::string_view>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(vec4& out, std::optional<vec4>&& v) { if (v.has_value()) { out = v.value(); } }

	template <typename BaseWriter, typename OutputStream>
	class WriterWrapper
	{
	public:
		WriterWrapper(OutputStream& os) : writer(os) { writer.SetIndent(IndentationChar, IndentationCharCount); }

		inline void ArrayBegin() { writer.StartArray(); }
		inline void ArrayEnd() { writer.EndArray(); }
		inline void MemberArrayBegin(std::string_view key) { MemberKey(key); ArrayBegin(); }
		inline void MemberArrayEnd() { ArrayEnd(); }

		inline void ObjectBegin() { writer.StartObject(); }
		inline void ObjectEnd() { writer.EndObject(); }
		inline void MemberObjectBegin(std::string_view key) { MemberKey(key); ObjectBegin(); }
		inline void MemberObjectEnd() { ObjectEnd(); }

		inline void Null() { writer.Null(); }
		inline void Bool(bool value) { writer.Bool(value); }
		inline void I32(i32 value) { writer.Int(value); }
		inline void U32(u32 value) { writer.Uint(value); }
		inline void F32(f32 value) { writer.Double(static_cast<f64>(value)); }
		inline void F64(f64 value) { writer.Double(value); }
		inline void Str(std::string_view value) { writer.String(value.data(), static_cast<SizeType>(value.size())); }
		template <typename Enum>
		inline void EnumStr(Enum value, const EnumNameMappingTable<Enum>& nameTable) { Str(EnumToStr(nameTable, value)); }

		inline void MemberKey(std::string_view key) { writer.Key(key.data(), static_cast<SizeType>(key.size())); }
		inline void MemberNull(std::string_view key) { MemberKey(key); Null(); }
		inline void MemberBool(std::string_view key, bool value) { MemberKey(key); Bool(value); }
		inline void MemberI32(std::string_view key, i32 value) { MemberKey(key); I32(value); }
		inline void MemberU32(std::string_view key, u32 value) { MemberKey(key); U32(value); }
		inline void MemberF32(std::string_view key, f32 value) { MemberKey(key); F32(value); }
		inline void MemberF64(std::string_view key, f64 value) { MemberKey(key); F64(value); }
		inline void MemberStr(std::string_view key, std::string_view value) { MemberKey(key); Str(value); }
		template <typename Enum>
		inline void MemberEnumStr(std::string_view key, Enum value, const EnumNameMappingTable<Enum>& nameTable) { MemberKey(key); EnumStr(value, nameTable); }

		inline void MemberVec4XYZW(std::string_view key, vec4 value) { MemberObjectBegin(key); MemberF32("x", value.x); MemberF32("y", value.y); MemberF32("z", value.z); MemberF32("w", value.w); MemberObjectEnd(); }
		inline void MemberVec4RGBA(std::string_view key, vec4 value) { MemberObjectBegin(key); MemberF32("r", value.r); MemberF32("g", value.g); MemberF32("b", value.b); MemberF32("a", value.a); MemberObjectEnd(); }

		inline void MemberHexRGBAStr(std::string_view key, u8 r, u8 g, u8 b, u8 a) { char buffer[10]; sprintf_s(buffer, "#%02X%02X%02X%02X", r, g, b, a); MemberStr(key, buffer); }
		inline void MemberHexRGBAStr(std::string_view key, u32 rgba) { MemberHexRGBAStr(key, (rgba >> 0) & 0xFF, (rgba >> 8) & 0xFF, (rgba >> 16) & 0xFF, (rgba >> 24) & 0xFF); }
		inline void MemberHexRGBAStr(std::string_view key, vec4 rgba) { auto toU8 = [](f32 c) { return static_cast<u8>(Clamp(c, 0.0f, 1.0f) * 255.0f + 0.5f); }; MemberHexRGBAStr(key, toU8(rgba.r), toU8(rgba.g), toU8(rgba.b), toU8(rgba.a)); }

		inline void MemberTryBool(std::string_view key, std::optional<bool> value) { MemberKey(key); value.has_value() ? Bool(value.value()) : Null(); }
		inline void MemberTryI32(std::string_view key, std::optional<i32> value) { MemberKey(key); value.has_value() ? I32(value.value()) : Null(); }
		inline void MemberTryU32(std::string_view key, std::optional<u32> value) { MemberKey(key); value.has_value() ? U32(value.value()) : Null(); }
		inline void MemberTryF32(std::string_view key, std::optional<f32> value) { MemberKey(key); value.has_value() ? F32(value.value()) : Null(); }
		inline void MemberTryF64(std::string_view key, std::optional<f64> value) { MemberKey(key); value.has_value() ? F64(value.value()) : Null(); }
		inline void MemberTryStr(std::string_view key, const std::optional<std::string>& value) { MemberKey(key); value.has_value() ? Str(value.value()) : Null(); }
		template <typename Enum>
		inline void MemberTryEnumStr(std::string_view key, const std::optional<Enum>& value, const EnumNameMappingTable<Enum>& nameTable) { MemberKey(key); value.has_value() ? EnumStr(value.value(), nameTable) : Null(); }

#if 1 // NOTE: To try and avoid implicit conversion error at compile time
		void MemberI32(std::string_view key, bool value) = delete;
		void MemberI32(std::string_view key, u32 value) = delete;
		void MemberI32(std::string_view key, f32 value) = delete;
		void MemberI32(std::string_view key, f64 value) = delete;

		void MemberU32(std::string_view key, bool value) = delete;
		void MemberU32(std::string_view key, i32 value) = delete;
		void MemberU32(std::string_view key, f32 value) = delete;
		void MemberU32(std::string_view key, f64 value) = delete;

		void MemberF32(std::string_view key, bool value) = delete;
		void MemberF32(std::string_view key, i32 value) = delete;
		void MemberF32(std::string_view key, u32 value) = delete;
		void MemberF32(std::string_view key, f64 value) = delete;

		void MemberF64(std::string_view key, bool value) = delete;
		void MemberF64(std::string_view key, i32 value) = delete;
		void MemberF64(std::string_view key, u32 value) = delete;
		void MemberF64(std::string_view key, f32 value) = delete;
#endif

	private:
		BaseWriter writer;
	};

	using WriteBuffer = rapidjson::StringBuffer;
	using WriterEx = WriterWrapper<rapidjson::PrettyWriter<WriteBuffer>, WriteBuffer>;
}
