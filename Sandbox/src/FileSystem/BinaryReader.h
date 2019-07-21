#pragma once
#include "Stream.h"
#include "BinaryMode.h"
#include <functional>
#include <intrin.h>

namespace FileSystem
{
	class BinaryReader
	{
	public:
		BinaryReader();
		BinaryReader(Stream* stream);
		~BinaryReader();

		void OpenStream(Stream* stream);
		void Close();

		bool IsOpen();
		bool GetLeaveOpen();

		inline int64_t GetPosition() const { return stream->GetPosition(); }
		inline void* GetPositionPtr() const { return (void*)stream->GetPosition(); }

		inline void SetPosition(int64_t position) { return stream->Seek(position); }
		inline void SetPosition(void* position) { return stream->Seek((int64_t)position); }

		inline int64_t GetLength() const { return stream->GetLength(); }
		inline bool EndOfFile() const { return stream->EndOfFile(); }

		PtrMode GetPointerMode() const;
		void SetPointerMode(PtrMode value);

		Endianness GetEndianness() const;
		void SetEndianness(Endianness value);

		inline int64_t Read(void* buffer, size_t size);

		void ReadAt(void* position, const std::function<void(BinaryReader&)>& func);

		template <typename T> T ReadAt(void* position, const std::function<T(BinaryReader&)>& func);

		inline void* ReadPtr() { return readPtrFunction(this); };

		std::string ReadStr();
		std::string ReadStrPtr();

		inline bool ReadBool() { return Read<bool>(); };
		inline char ReadChar() { return Read<char>(); };

		inline uint8_t ReadInt8() { return Read<int8_t>(); };
		inline uint8_t ReadUInt8() { return Read<uint8_t>(); };

		inline int16_t ReadInt16() { return readInt16Function(this); };
		inline uint16_t ReadUInt16() { return readUInt16Function(this); };
		inline int32_t ReadInt32() { return readInt32Function(this); };
		inline uint32_t ReadUInt32() { return readUInt32Function(this); };
		inline int64_t ReadInt64() { return readInt64Function(this); };
		inline uint64_t ReadUInt64() { return readUInt64Function(this); };
		inline float ReadFloat() { return readFloatFunction(this); };
		inline double ReadDouble() { return readDoubleFunction(this); };

	protected:
		typedef void* (*ReadPtr_t)(BinaryReader*);
		typedef int16_t (*ReadInt16_t)(BinaryReader*);
		typedef uint16_t (*ReadUInt16_t)(BinaryReader*);
		typedef int32_t (*ReadInt32_t)(BinaryReader*);
		typedef uint32_t (*ReadUInt32_t)(BinaryReader*);
		typedef int64_t (*ReadInt64_t)(BinaryReader*);
		typedef uint64_t (*ReadUInt64_t)(BinaryReader*);
		typedef float (*ReadFloat_t)(BinaryReader*);
		typedef double (*ReadDouble_t)(BinaryReader*);

		ReadPtr_t readPtrFunction = nullptr;
		ReadInt16_t readInt16Function = nullptr;
		ReadUInt16_t readUInt16Function = nullptr;
		ReadInt32_t readInt32Function = nullptr;
		ReadUInt32_t readUInt32Function = nullptr;
		ReadInt64_t readInt64Function = nullptr;
		ReadUInt64_t readUInt64Function = nullptr;
		ReadFloat_t readFloatFunction = nullptr;
		ReadDouble_t readDoubleFunction = nullptr;

		PtrMode pointerMode;
		Endianness endianness;

		bool leaveOpen = false;
		Stream* stream = nullptr;

	private:
		template <typename T> T Read()
		{
			T value;
			Read(&value, sizeof(value));
			return value;
		};

		static void* Read32BitPtr(BinaryReader* reader) { return reinterpret_cast<void*>(static_cast<ptrdiff_t>(reader->ReadInt32())); };
		static void* Read64BitPtr(BinaryReader* reader) { return reinterpret_cast<void*>(static_cast<ptrdiff_t>(reader->ReadInt64())); };

		static int16_t LE_ReadInt16(BinaryReader* reader) { return reader->Read<int16_t>(); };
		static uint16_t LE_ReadUInt16(BinaryReader* reader) { return reader->Read<uint16_t>(); };
		static int32_t LE_ReadInt32(BinaryReader* reader) { return reader->Read<int32_t>(); };
		static uint32_t LE_ReadUInt32(BinaryReader* reader) { return reader->Read<uint32_t>(); };
		static int64_t LE_ReadInt64(BinaryReader* reader) { return reader->Read<int64_t>(); };
		static uint64_t LE_ReadUInt64(BinaryReader* reader) { return reader->Read<uint64_t>(); };
		static float LE_ReadFloat(BinaryReader* reader) { return reader->Read<float>(); };
		static double LE_ReadDouble(BinaryReader* reader) { return reader->Read<double>(); };

		static int16_t BE_ReadInt16(BinaryReader* reader) { return _byteswap_ushort(reader->Read<int16_t>()); };
		static uint16_t BE_ReadUInt16(BinaryReader* reader) { return _byteswap_ushort(reader->Read<uint16_t>()); };
		static int32_t BE_ReadInt32(BinaryReader* reader) { return _byteswap_ulong(reader->Read<int32_t>()); };
		static uint32_t BE_ReadUInt32(BinaryReader* reader) { return _byteswap_ulong(reader->Read<uint32_t>()); };
		static int64_t BE_ReadInt64(BinaryReader* reader) { return _byteswap_uint64(reader->Read<int64_t>()); };
		static uint64_t BE_ReadUInt64(BinaryReader* reader) { return _byteswap_uint64(reader->Read<uint64_t>()); };
		static float BE_ReadFloat(BinaryReader* reader) { uint32_t value = _byteswap_ulong(reader->Read<uint32_t>()); return *(float*)&value; };
		static double BE_ReadDouble(BinaryReader* reader) { uint64_t value = _byteswap_uint64(reader->Read<uint64_t>()); return *(double*)&value; };
	};

	template <typename T>
	T BinaryReader::ReadAt(void* position, const std::function<T(BinaryReader&)>& func)
	{
		int64_t prePos = GetPosition();
		SetPosition(position);
		T value = func(*this);
		SetPosition(prePos);

		return value;
	}
}
