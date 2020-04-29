#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Stream/Stream.h"
#include "BinaryMode.h"
#include <list>
#include <map>
#include <functional>

namespace Comfy::IO
{
	class StreamWriter : NonCopyable
	{
	public:
		static constexpr u32 PaddingValue = 0xCCCCCCCC;

	public:
		StreamWriter()
		{
			SetPointerMode(PtrMode::Mode32Bit);
		}

		explicit StreamWriter(IStream& stream) : StreamWriter()
		{
			OpenStream(stream);
		}

		~StreamWriter()
		{
			Close();
		}

		inline void OpenStream(IStream& stream)
		{
			assert(stream.CanWrite() && underlyingStream == nullptr);
			underlyingStream = &stream;
		}

		inline void Close()
		{
			if (underlyingStream != nullptr && !leaveStreamOpen)
				underlyingStream->Close();
			underlyingStream = nullptr;
		}

		inline bool IsOpen() const { return underlyingStream != nullptr; }
		inline bool GetLeaveStreamOpen() const { return leaveStreamOpen; }

		inline FileAddr GetPosition() const { return underlyingStream->GetPosition(); }
		inline void SetPosition(FileAddr position) { return underlyingStream->Seek(position); }

		inline void SkipPosition(FileAddr increment) { return underlyingStream->Seek(GetPosition() + increment); }

		inline FileAddr GetLength() const { return underlyingStream->GetLength(); }
		inline bool EndOfFile() const { return underlyingStream->EndOfFile(); }

		inline PtrMode GetPointerMode() const { return pointerMode; }
		void SetPointerMode(PtrMode mode);

		inline size_t WriteBuffer(const void* buffer, size_t size) { return underlyingStream->WriteBuffer(buffer, size); }

		template <typename T>
		void WriteType(T value) { WriteBuffer(&value, sizeof(T)); }

		void WriteStr(std::string_view value);
		void WriteStrPtr(std::string_view value, i32 alignment = 0);

		inline void WritePtr(FileAddr value) { writePtrFunc(*this, value); };
		inline void WritePtr(nullptr_t) = delete;

		void WritePtr(const std::function<void(StreamWriter&)>& func, FileAddr baseAddress = FileAddr::NullPtr);
		void WriteDelayedPtr(const std::function<void(StreamWriter&)>& func);

		void WritePadding(size_t size, u32 paddingValue = PaddingValue);
		void WriteAlignmentPadding(i32 alignment, u32 paddingValue = PaddingValue);

		void FlushStringPointerPool();
		void FlushPointerPool();
		void FlushDelayedWritePool();

		inline void WriteBool(bool value) { return WriteType<bool>(value); }
		inline void WriteChar(char value) { return WriteType<char>(value); }
		inline void WriteI8(i8 value) { return WriteType<i8>(value); }
		inline void WriteU8(u8 value) { return WriteType<u8>(value); }
		inline void WriteI16(i16 value) { return WriteType<i16>(value); }
		inline void WriteU16(u16 value) { return WriteType<u16>(value); }
		inline void WriteI32(i32 value) { return WriteType<i32>(value); }
		inline void WriteU32(u32 value) { return WriteType<u32>(value); }
		inline void WriteI64(i64 value) { return WriteType<i64>(value); }
		inline void WriteU64(u64 value) { return WriteType<u64>(value); }
		inline void WriteF32(float value) { return WriteType<float>(value); }
		inline void WriteF64(double value) { return WriteType<double>(value); }

	protected:
		using WritePtrFunc_t = void(StreamWriter&, FileAddr);
		WritePtrFunc_t* writePtrFunc = nullptr;

		PtrMode pointerMode = {};
		bool leaveStreamOpen = false;
		bool poolStrings = true;
		IStream* underlyingStream = nullptr;

		struct StringPointerEntry
		{
			FileAddr ReturnAddress;
			std::string_view String;
			i32 Alignment;
		};

		struct FunctionPointerEntry
		{
			FileAddr ReturnAddress;
			FileAddr BaseAddress;
			const std::function<void(StreamWriter&)> Function;
		};

		struct DelayedWriteEntry
		{
			FileAddr ReturnAddress;
			const std::function<void(StreamWriter&)> Function;
		};

		std::unordered_map<std::string, FileAddr> writtenStringPool;
		std::vector<StringPointerEntry> stringPointerPool;
		std::list<FunctionPointerEntry> pointerPool;
		std::vector<DelayedWriteEntry> delayedWritePool;

	private:
		static void WritePtr32(StreamWriter& writer, FileAddr value) { writer.WriteI32(static_cast<i32>(value)); }
		static void WritePtr64(StreamWriter& writer, FileAddr value) { writer.WriteI64(static_cast<i64>(value)); }
	};
}
