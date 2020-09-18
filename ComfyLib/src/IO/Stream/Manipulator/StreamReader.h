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

	public:
		std::string ReadStr();
		std::string ReadStrAtOffsetAware(FileAddr position);
		std::string ReadStr(size_t size);
		std::string ReadStrPtrOffsetAware();

		// TODO: Return StreamResult instead of void and automatically return StreamResult::BadPointer if the input position is invalid (?)

		template <typename Func>
		void ReadAt(FileAddr position, Func func)
		{
			const auto prePos = GetPosition();
			Seek(position);
			func(*this);
			Seek(prePos);
		}

		template <typename Func>
		void ReadAtOffsetAware(FileAddr position, Func func)
		{
			ReadAt(position + baseOffset, func);
		}

		template <typename T, typename Func>
		T ReadValueAt(FileAddr position, Func func)
		{
			const auto prePos = GetPosition();
			Seek(position);
			const T value = func(*this);
			Seek(prePos);
			return value;
		}

		template <typename T, typename Func>
		T ReadValueAtOffsetAware(FileAddr position, Func func)
		{
			return ReadValueAt<T>(position + baseOffset, func);
		}

		inline bool IsValidPointer(FileAddr address, bool offsetAware = true)
		{
			return (address > FileAddr::NullPtr) && ((offsetAware ? address + baseOffset : address) <= underlyingStream->GetLength());
		}

		void SeekAlign(i32 alignment);

		inline bool GetWriteEmptyNullStrPtr() const { return settings.EmptyNullStringPointers; }
		inline void SetWriteEmptyNullStrPtr(bool value) { settings.EmptyNullStringPointers = value; }

	public:
		inline FileAddr ReadPtr() { return (this->*readPtrFunc)(); }
		inline size_t ReadSize() { return (this->*readSizeFunc)(); }
		inline bool ReadBool() { return ReadType_Native<bool>(); }
		inline char ReadChar() { return ReadType_Native<char>(); }
		inline i8 ReadI8() { return ReadType_Native<i8>(); }
		inline u8 ReadU8() { return ReadType_Native<u8>(); }
		inline i16 ReadI16() { return (this->*readI16Func)(); }
		inline u16 ReadU16() { return (this->*readU16Func)(); }
		inline i32 ReadI32() { return (this->*readI32Func)(); }
		inline u32 ReadU32() { return (this->*readU32Func)(); }
		inline i64 ReadI64() { return (this->*readI64Func)(); }
		inline u64 ReadU64() { return (this->*readU64Func)(); }
		inline f32 ReadF32() { return (this->*readF32Func)(); }
		inline f64 ReadF64() { return (this->*readF64Func)(); }

	public:
		inline vec2 ReadV2() { vec2 result; result.x = ReadF32(); result.y = ReadF32(); return result; }
		inline vec3 ReadV3() { vec3 result; result.x = ReadF32(); result.y = ReadF32(); result.z = ReadF32(); return result; }
		inline vec4 ReadV4() { vec4 result; result.x = ReadF32(); result.y = ReadF32(); result.z = ReadF32(); result.w = ReadF32(); return result; }
		inline mat3 ReadMat3() { assert(GetEndianness() == Endianness::Native); return ReadType_Native<mat3>(); }
		inline mat4 ReadMat4() { assert(GetEndianness() == Endianness::Native); return ReadType_Native<mat4>(); }
		inline ivec2 ReadIV2() { ivec2 result; result.x = ReadI32(); result.y = ReadI32(); return result; }
		inline ivec3 ReadIV3() { ivec3 result; result.x = ReadI32(); result.y = ReadI32(); result.z = ReadI32(); return result; }
		inline ivec4 ReadIV4() { ivec4 result; result.x = ReadI32(); result.y = ReadI32(); result.z = ReadI32(); result.w = ReadI32(); return result; }

	public:
		template <typename T>
		T ReadType_Native() { T value; ReadBuffer(&value, sizeof(value)); return value; }

		inline FileAddr ReadPtr_32() { return static_cast<FileAddr>(ReadI32()); }
		inline FileAddr ReadPtr_64() { return static_cast<FileAddr>(ReadI64()); }

		inline size_t ReadSize_32() { return static_cast<size_t>(ReadU32()); }
		inline size_t ReadSize_64() { return static_cast<size_t>(ReadU64()); }

		inline i16 ReadI16_LE() { return ReadType_Native<i16>(); }
		inline u16 ReadU16_LE() { return ReadType_Native<u16>(); }
		inline i32 ReadI32_LE() { return ReadType_Native<i32>(); }
		inline u32 ReadU32_LE() { return ReadType_Native<u32>(); }
		inline i64 ReadI64_LE() { return ReadType_Native<i64>(); }
		inline u64 ReadU64_LE() { return ReadType_Native<u64>(); }
		inline f32 ReadF32_LE() { return ReadType_Native<f32>(); }
		inline f64 ReadF64_LE() { return ReadType_Native<f64>(); }

		inline i16 ReadI16_BE() { return Util::ByteSwapI16(ReadI16_LE()); }
		inline u16 ReadU16_BE() { return Util::ByteSwapU16(ReadU16_LE()); }
		inline i32 ReadI32_BE() { return Util::ByteSwapI32(ReadI32_LE()); }
		inline u32 ReadU32_BE() { return Util::ByteSwapU32(ReadU32_LE()); }
		inline i64 ReadI64_BE() { return Util::ByteSwapI64(ReadI64_LE()); }
		inline u64 ReadU64_BE() { return Util::ByteSwapU64(ReadU64_LE()); }
		inline f32 ReadF32_BE() { return Util::ByteSwapF32(ReadF32_LE()); }
		inline f64 ReadF64_BE() { return Util::ByteSwapF64(ReadF64_LE()); }

	protected:
		void OnPointerModeChanged() override;
		void OnEndiannessChanged() override;

	private:
		FileAddr(StreamReader::*readPtrFunc)() = nullptr;
		size_t(StreamReader::*readSizeFunc)() = nullptr;
		i16(StreamReader::*readI16Func)() = nullptr;
		u16(StreamReader::*readU16Func)() = nullptr;
		i32(StreamReader::*readI32Func)() = nullptr;
		u32(StreamReader::*readU32Func)() = nullptr;
		i64(StreamReader::*readI64Func)() = nullptr;
		u64(StreamReader::*readU64Func)() = nullptr;
		f32(StreamReader::*readF32Func)() = nullptr;
		f64(StreamReader::*readF64Func)() = nullptr;

		struct Settings
		{
			bool EmptyNullStringPointers = false;
		} settings;
	};
}
