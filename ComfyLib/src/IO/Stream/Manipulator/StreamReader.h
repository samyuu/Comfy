#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "StreamManipulator.h"

namespace Comfy::IO
{
	class StreamReader final : public StreamManipulator, NonCopyable
	{
	public:
		StreamReader(IStream& stream) : StreamManipulator(stream)
		{
			assert(stream.CanRead());
			OnPointerModeChanged();
			OnEndiannessChanged();
		}

		~StreamReader() = default;

	public:
		inline size_t ReadBuffer(void* buffer, size_t size) { return underlyingStream->ReadBuffer(buffer, size); }

		template <typename T>
		T ReadType() { T value; ReadBuffer(&value, sizeof(value)); return value; }

		inline FileAddr GetStreamSeekOffset() const { return streamSeekOffset; }
		inline void SetStreamSeekOffset(FileAddr value) { streamSeekOffset = value; }

		template <typename Func>
		void ReadAt(FileAddr position, const Func func)
		{
			const auto prePos = GetPosition();
			SetPosition(position);
			func(*this);
			SetPosition(prePos);
		}

		template <typename Func>
		void ReadAt(FileAddr position, FileAddr baseAddress, Func func)
		{
			ReadAt((position + baseAddress), func);
		}

		template <typename T, typename Func>
		T ReadAt(FileAddr position, Func func)
		{
			const auto prePos = GetPosition();
			SetPosition(position);
			const T value = func(*this);
			SetPosition(prePos);
			return value;
		}

		std::string ReadStr();
		std::string ReadStrAt(FileAddr position);
		std::string ReadStr(size_t size);

		inline std::string ReadStrPtr() { return ReadStrAt(ReadPtr()); }

		inline FileAddr ReadPtr() { return readPtrFunc(*this); }
		inline size_t ReadSize() { return readSizeFunc(*this); }
		inline bool ReadBool() { return ReadType<bool>(); }
		inline char ReadChar() { return ReadType<char>(); }
		inline i8 ReadI8() { return ReadType<i8>(); }
		inline u8 ReadU8() { return ReadType<u8>(); }
		inline i16 ReadI16() { return readI16Func(*this); }
		inline u16 ReadU16() { return readU16Func(*this); }
		inline i32 ReadI32() { return readI32Func(*this); }
		inline u32 ReadU32() { return readU32Func(*this); }
		inline i64 ReadI64() { return readI64Func(*this); }
		inline u64 ReadU64() { return readU64Func(*this); }
		inline f32 ReadF32() { return readF32Func(*this); }
		inline f64 ReadF64() { return readF64Func(*this); }

		inline vec2 ReadV2() { vec2 result; result.x = ReadF32(); result.y = ReadF32(); return result; }
		inline vec3 ReadV3() { vec3 result; result.x = ReadF32(); result.y = ReadF32(); result.z = ReadF32(); return result; }
		inline vec4 ReadV4() { vec4 result; result.x = ReadF32(); result.y = ReadF32(); result.z = ReadF32(); result.w = ReadF32(); return result; }
		inline mat3 ReadMat3() { assert(GetEndianness() == Endianness::Native); return ReadType<mat3>(); }
		inline mat4 ReadMat4() { assert(GetEndianness() == Endianness::Native); return ReadType<mat4>(); }
		inline ivec2 ReadIV2() { ivec2 result; result.x = ReadI32(); result.y = ReadI32(); return result; }
		inline ivec3 ReadIV3() { ivec3 result; result.x = ReadI32(); result.y = ReadI32(); result.z = ReadI32(); return result; }
		inline ivec4 ReadIV4() { ivec4 result; result.x = ReadI32(); result.y = ReadI32(); result.z = ReadI32(); result.w = ReadI32(); return result; }

	protected:
		void OnPointerModeChanged() override;
		void OnEndiannessChanged() override;

	private:
		using ReadPtrFunc = FileAddr(StreamReader&);
		using ReadSizeFunc = size_t(StreamReader&);
		using ReadI16Func = i16(StreamReader&);
		using ReadU16Func = u16(StreamReader&);
		using ReadI32Func = i32(StreamReader&);
		using ReadU32Func = u32(StreamReader&);
		using ReadI64Func = i64(StreamReader&);
		using ReadU64Func = u64(StreamReader&);
		using ReadF32Func = f32(StreamReader&);
		using ReadF64Func = f64(StreamReader&);

		ReadPtrFunc* readPtrFunc = nullptr;
		ReadSizeFunc* readSizeFunc = nullptr;
		ReadI16Func* readI16Func = nullptr;
		ReadU16Func* readU16Func = nullptr;
		ReadI32Func* readI32Func = nullptr;
		ReadU32Func* readU32Func = nullptr;
		ReadI64Func* readI64Func = nullptr;
		ReadU64Func* readU64Func = nullptr;
		ReadF32Func* readF32Func = nullptr;
		ReadF64Func* readF64Func = nullptr;

		// TODO: Remove (?)
		FileAddr streamSeekOffset = {};

	private:
		static FileAddr ReadPtr32(StreamReader& reader) { return static_cast<FileAddr>(reader.ReadI32()); }
		static FileAddr ReadPtr64(StreamReader& reader) { return static_cast<FileAddr>(reader.ReadI64()); }

		static size_t ReadSize32(StreamReader& reader) { return static_cast<size_t>(reader.ReadU32()); }
		static size_t ReadSize64(StreamReader& reader) { return static_cast<size_t>(reader.ReadU64()); }

		static i16 LE_ReadI16(StreamReader& reader) { return reader.ReadType<i16>(); }
		static u16 LE_ReadU16(StreamReader& reader) { return reader.ReadType<u16>(); }
		static i32 LE_ReadI32(StreamReader& reader) { return reader.ReadType<i32>(); }
		static u32 LE_ReadU32(StreamReader& reader) { return reader.ReadType<u32>(); }
		static i64 LE_ReadI64(StreamReader& reader) { return reader.ReadType<i64>(); }
		static u64 LE_ReadU64(StreamReader& reader) { return reader.ReadType<u64>(); }
		static f32 LE_ReadF32(StreamReader& reader) { return reader.ReadType<f32>(); }
		static f64 LE_ReadF64(StreamReader& reader) { return reader.ReadType<f64>(); }

		static i16 BE_ReadI16(StreamReader& reader) { return Utilities::ByteSwapI16(reader.ReadType<i16>()); }
		static u16 BE_ReadU16(StreamReader& reader) { return Utilities::ByteSwapU16(reader.ReadType<u16>()); }
		static i32 BE_ReadI32(StreamReader& reader) { return Utilities::ByteSwapI32(reader.ReadType<i32>()); }
		static u32 BE_ReadU32(StreamReader& reader) { return Utilities::ByteSwapU32(reader.ReadType<u32>()); }
		static i64 BE_ReadI64(StreamReader& reader) { return Utilities::ByteSwapI64(reader.ReadType<i64>()); }
		static u64 BE_ReadU64(StreamReader& reader) { return Utilities::ByteSwapU64(reader.ReadType<u64>()); }
		static f32 BE_ReadF32(StreamReader& reader) { return Utilities::ByteSwapF32(reader.ReadType<f32>()); }
		static f64 BE_ReadF64(StreamReader& reader) { return Utilities::ByteSwapF64(reader.ReadType<f64>()); }
	};
}
