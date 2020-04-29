#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Stream/Stream.h"
#include "BinaryMode.h"
#include "Misc/EndianHelper.h"

namespace Comfy::IO
{
	class StreamReader : NonCopyable
	{
	public:
		StreamReader()
		{
			SetPointerMode(PtrMode::Mode32Bit);
			SetEndianness(Endianness::Little);
		}

		StreamReader(IStream& stream) : StreamReader()
		{
			OpenStream(stream);
		}

		~StreamReader()
		{
			Close();
		}

		inline void OpenStream(IStream& stream)
		{
			assert(stream.CanRead() && underlyingStream == nullptr);
			underlyingStream = &stream;
		}

		inline void Close()
		{
			if (underlyingStream != nullptr && !leaveStreamOpen)
				underlyingStream->Close();
			underlyingStream = nullptr;
		}

		inline bool IsOpen() const { return underlyingStream != nullptr; }

		inline void SetLeaveStreamOpen(bool value) { leaveStreamOpen = value; }
		inline bool GetLeaveStreamOpen() const { return leaveStreamOpen; }

		inline FileAddr GetPosition() const { return underlyingStream->GetPosition() - streamSeekOffset; }
		inline void SetPosition(FileAddr position) { return underlyingStream->Seek(position + streamSeekOffset); }

		inline FileAddr GetLength() const { return underlyingStream->GetLength(); }
		inline bool EndOfFile() const { return underlyingStream->EndOfFile(); }

		inline FileAddr GetStreamSeekOffset() const { return streamSeekOffset; }
		inline void SetStreamSeekOffset(FileAddr value) { streamSeekOffset = value; }

		inline PtrMode GetPointerMode() const { return pointerMode; }
		void SetPointerMode(PtrMode value);

		inline Endianness GetEndianness() const { return endianness; }
		void SetEndianness(Endianness value);

		inline size_t ReadBuffer(void* buffer, size_t size) { return underlyingStream->ReadBuffer(buffer, size); }

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
		inline int64_t ReadI64() { return readI64Func(*this); }
		inline u64 ReadU64() { return readU64Func(*this); }
		inline float ReadF32() { return readF32Func(*this); }
		inline double ReadF64() { return readF64Func(*this); }

		inline vec2 ReadV2() { vec2 result; result.x = ReadF32(); result.y = ReadF32(); return result; }
		inline vec3 ReadV3() { vec3 result; result.x = ReadF32(); result.y = ReadF32(); result.z = ReadF32(); return result; }
		inline vec4 ReadV4() { vec4 result; result.x = ReadF32(); result.y = ReadF32(); result.z = ReadF32(); result.w = ReadF32(); return result; }
		inline mat3 ReadMat3() { assert(GetEndianness() == Endianness::Native); return ReadType<mat3>(); }
		inline mat4 ReadMat4() { assert(GetEndianness() == Endianness::Native); return ReadType<mat4>(); }
		inline ivec2 ReadIV2() { ivec2 result; result.x = ReadI32(); result.y = ReadI32(); return result; }
		inline ivec3 ReadIV3() { ivec3 result; result.x = ReadI32(); result.y = ReadI32(); result.z = ReadI32(); return result; }
		inline ivec4 ReadIV4() { ivec4 result; result.x = ReadI32(); result.y = ReadI32(); result.z = ReadI32(); result.w = ReadI32(); return result; }

	protected:
		using ReadPtrFunc_t = FileAddr(StreamReader&);
		using ReadSizeFunc_t = size_t(StreamReader&);
		using ReadI16Func_t = i16(StreamReader&);
		using ReadU16Func_t = u16(StreamReader&);
		using ReadI32Func_t = i32(StreamReader&);
		using ReadU32Func_t = u32(StreamReader&);
		using ReadI64Func_t = int64_t(StreamReader&);
		using ReadU64Func_t = u64(StreamReader&);
		using ReadF32Func_t = float(StreamReader&);
		using ReadF64Func_t = double(StreamReader&);

		ReadPtrFunc_t* readPtrFunc = nullptr;
		ReadSizeFunc_t* readSizeFunc = nullptr;
		ReadI16Func_t* readI16Func = nullptr;
		ReadU16Func_t* readU16Func = nullptr;
		ReadI32Func_t* readI32Func = nullptr;
		ReadU32Func_t* readU32Func = nullptr;
		ReadI64Func_t* readI64Func = nullptr;
		ReadU64Func_t* readU64Func = nullptr;
		ReadF32Func_t* readF32Func = nullptr;
		ReadF64Func_t* readF64Func = nullptr;

		PtrMode pointerMode = PtrMode::Mode32Bit;
		Endianness endianness = Endianness::Little;

		bool leaveStreamOpen = false;
		FileAddr streamSeekOffset = {};
		IStream* underlyingStream = nullptr;

	private:
		template <typename T>
		T ReadType()
		{
			T value;
			ReadBuffer(&value, sizeof(value));
			return value;
		}

		static FileAddr ReadPtr32(StreamReader& reader) { return static_cast<FileAddr>(reader.ReadI32()); }
		static FileAddr ReadPtr64(StreamReader& reader) { return static_cast<FileAddr>(reader.ReadI64()); }

		static size_t ReadSize32(StreamReader& reader) { return static_cast<size_t>(reader.ReadU32()); }
		static size_t ReadSize64(StreamReader& reader) { return static_cast<size_t>(reader.ReadU64()); }

		static i16 LE_ReadI16(StreamReader& reader) { return reader.ReadType<i16>(); }
		static u16 LE_ReadU16(StreamReader& reader) { return reader.ReadType<u16>(); }
		static i32 LE_ReadI32(StreamReader& reader) { return reader.ReadType<i32>(); }
		static u32 LE_ReadU32(StreamReader& reader) { return reader.ReadType<u32>(); }
		static int64_t LE_ReadI64(StreamReader& reader) { return reader.ReadType<int64_t>(); }
		static u64 LE_ReadU64(StreamReader& reader) { return reader.ReadType<u64>(); }
		static float LE_ReadF32(StreamReader& reader) { return reader.ReadType<float>(); }
		static double LE_ReadF64(StreamReader& reader) { return reader.ReadType<double>(); }

		static i16 BE_ReadI16(StreamReader& reader) { return Utilities::ByteSwapI16(reader.ReadType<i16>()); }
		static u16 BE_ReadU16(StreamReader& reader) { return Utilities::ByteSwapU16(reader.ReadType<u16>()); }
		static i32 BE_ReadI32(StreamReader& reader) { return Utilities::ByteSwapI32(reader.ReadType<i32>()); }
		static u32 BE_ReadU32(StreamReader& reader) { return Utilities::ByteSwapU32(reader.ReadType<u32>()); }
		static int64_t BE_ReadI64(StreamReader& reader) { return Utilities::ByteSwapI64(reader.ReadType<int64_t>()); }
		static u64 BE_ReadU64(StreamReader& reader) { return Utilities::ByteSwapU64(reader.ReadType<u64>()); }
		static float BE_ReadF32(StreamReader& reader) { return Utilities::ByteSwapF32(reader.ReadType<float>()); }
		static double BE_ReadF64(StreamReader& reader) { return Utilities::ByteSwapF64(reader.ReadType<double>()); }
	};
}
