#pragma once
#include "Stream.h"
#include "PtrMode.h"
#include <functional>

//#define BIG_ENDIAN
#define LITTLE_ENDIAN
#if defined (BIG_ENDIAN)
#include <intrin.h>
#endif

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
		void SetPointerMode(PtrMode mode);

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

#if defined(LITTLE_ENDIAN)
		inline int16_t ReadInt16() { return Read<int16_t>(); };
		inline uint16_t ReadUInt16() { return Read<uint16_t>(); };
		inline int32_t ReadInt32() { return Read<int32_t>(); };
		inline uint32_t ReadUInt32() { return Read<uint32_t>(); };
		inline int64_t ReadInt64() { return Read<int64_t>(); };
		inline uint64_t ReadUInt64() { return Read<uint64_t>(); };
		inline float ReadFloat() { return Read<float>(); };
		inline double ReadDouble() { return Read<double>(); };

#elif defined (BIG_ENDIAN)
		inline int16_t ReadInt16() { return _byteswap_ushort(Read<int16_t>()); };
		inline int16_t ReadUInt16() { return _byteswap_ushort(Read<uint16_t>()); };

		inline int32_t ReadInt32() { return _byteswap_ulong(Read<int32_t>()); };
		inline uint32_t ReadUInt32() { return _byteswap_ulong(Read<uint32_t>()); };

		inline int64_t ReadInt64() { return _byteswap_uint64(Read<int64_t>()); };
		inline uint64_t ReadUInt64() { return _byteswap_uint64(Read<uint64_t>()); };

		inline float ReadFloat() { uint32_t value = _byteswap_ulong(Read<uint32_t>()); return *(float*)&value; };
		inline double ReadDouble() { uint64_t value = _byteswap_uint64(Read<uint64_t>()); return *(double*)&value; };
#endif

	protected:
		typedef void* (*ReadPtr_t)(BinaryReader*);
		ReadPtr_t readPtrFunction = nullptr;

		PtrMode pointerMode;
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
