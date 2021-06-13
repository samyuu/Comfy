#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CoreMacros.h"
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

#if 0 // TODO: Which to use..?
	constexpr i32 IndentationCharCount = 4;
	constexpr char IndentationChar = ' ';
#else // ... these very .cpp source files use "\r\n" (0x0D, 0x0A) for new lines and a single "\t" (0x09) as indentation rendered as 4 spaces
	constexpr i32 IndentationCharCount = 1;
	constexpr char IndentationChar = '\t';
#endif

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

	inline void TryAssign(bool& out, std::optional<bool>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(i32& out, std::optional<i32>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(u32& out, std::optional<u32>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(f32& out, std::optional<f32>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(f64& out, std::optional<f64>&& v) { if (v.has_value()) { out = v.value(); } }
	inline void TryAssign(std::string& out, std::optional<std::string_view>&& v) { if (v.has_value()) { out = v.value(); } }

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

		inline void MemberKey(std::string_view key) { writer.Key(key.data(), static_cast<SizeType>(key.size())); }
		inline void MemberNull(std::string_view key) { MemberKey(key); Null(); }
		inline void MemberBool(std::string_view key, bool value) { MemberKey(key); Bool(value); }
		inline void MemberI32(std::string_view key, i32 value) { MemberKey(key); I32(value); }
		inline void MemberU32(std::string_view key, u32 value) { MemberKey(key); U32(value); }
		inline void MemberF32(std::string_view key, f32 value) { MemberKey(key); F32(value); }
		inline void MemberF64(std::string_view key, f64 value) { MemberKey(key); F64(value); }
		inline void MemberStr(std::string_view key, std::string_view value) { MemberKey(key); Str(value); }

		// TODO: Consider renaming all optional helpers from "Try" to "Opt" (?)
		inline void MemberTryBool(std::string_view key, std::optional<bool> value) { MemberKey(key); value.has_value() ? Bool(value.value()) : Null(); }
		inline void MemberTryI32(std::string_view key, std::optional<i32> value) { MemberKey(key); value.has_value() ? I32(value.value()) : Null(); }
		inline void MemberTryU32(std::string_view key, std::optional<u32> value) { MemberKey(key); value.has_value() ? U32(value.value()) : Null(); }
		inline void MemberTryF32(std::string_view key, std::optional<f32> value) { MemberKey(key); value.has_value() ? F32(value.value()) : Null(); }
		inline void MemberTryF64(std::string_view key, std::optional<f64> value) { MemberKey(key); value.has_value() ? F64(value.value()) : Null(); }
		inline void MemberTryStr(std::string_view key, const std::optional<std::string>& value) { MemberKey(key); value.has_value() ? Str(value.value()) : Null(); }

#if 1 // TODO: Is this a good idea (?)
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

	void DummyFunctionToShutUpLinkerWarning4221();
}
