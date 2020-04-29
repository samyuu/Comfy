#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "StreamManipulator.h"
#include <list>
#include <map>
#include <functional>

namespace Comfy::IO
{
	class StreamWriter final : public StreamManipulator, NonCopyable
	{
	public:
		explicit StreamWriter(IStream& stream) : StreamManipulator(stream)
		{
			assert(stream.CanWrite());
			OnPointerModeChanged();
			OnEndiannessChanged();
		}

		~StreamWriter() = default;

	public:
		inline size_t WriteBuffer(const void* buffer, size_t size) { return underlyingStream->WriteBuffer(buffer, size); }

	public:
		void WriteStr(std::string_view value);
		void WriteStrPtr(std::string_view value, i32 alignment = 0);

		void WriteFuncPtr(const std::function<void(StreamWriter&)>& func, FileAddr baseAddress = FileAddr::NullPtr);
		void WriteDelayedPtr(const std::function<void(StreamWriter&)>& func);

		void WritePadding(size_t size, u32 paddingValue = PaddingValue);
		void WriteAlignmentPadding(i32 alignment, u32 paddingValue = PaddingValue);

	public:
		void FlushStringPointerPool();
		void FlushPointerPool();
		void FlushDelayedWritePool();

	public:
		inline void WritePtr(FileAddr value) { (this->*writePtrFunc)(value); }
		inline void WriteSize(size_t value) { (this->*writeSizeFunc)(value); }
		inline void WriteBool(bool value) { WriteType_Native<bool>(value); }
		inline void WriteChar(char value) { WriteType_Native<char>(value); }
		inline void WriteI8(i8 value) { WriteType_Native<i8>(value); }
		inline void WriteU8(u8 value) { WriteType_Native<u8>(value); }
		inline void WriteI16(i16 value) { (this->*writeI16Func)(value); }
		inline void WriteU16(u16 value) { (this->*writeU16Func)(value); }
		inline void WriteI32(i32 value) { (this->*writeI32Func)(value); }
		inline void WriteU32(u32 value) { (this->*writeU32Func)(value); }
		inline void WriteI64(i64 value) { (this->*writeI64Func)(value); }
		inline void WriteU64(u64 value) { (this->*writeU64Func)(value); }
		inline void WriteF32(f32 value) { (this->*writeF32Func)(value); }
		inline void WriteF64(f64 value) { (this->*writeF64Func)(value); }

	public:
		template <typename T>
		void WriteType_Native(T value) { WriteBuffer(&value, sizeof(T)); }

		inline void WritePtr_32(FileAddr value) { WriteI32(static_cast<i32>(value)); }
		inline void WritePtr_64(FileAddr value) { WriteI64(static_cast<i64>(value)); }

		inline void WriteSize_32(size_t value) { WriteU32(static_cast<u32>(value)); }
		inline void WriteSize_64(size_t value) { WriteU64(static_cast<u64>(value)); }

		inline void WriteI16_LE(i16 value) { WriteType_Native<i16>(value); }
		inline void WriteU16_LE(u16 value) { WriteType_Native<u16>(value); }
		inline void WriteI32_LE(i32 value) { WriteType_Native<i32>(value); }
		inline void WriteU32_LE(u32 value) { WriteType_Native<u32>(value); }
		inline void WriteI64_LE(i64 value) { WriteType_Native<i64>(value); }
		inline void WriteU64_LE(u64 value) { WriteType_Native<u64>(value); }
		inline void WriteF32_LE(f32 value) { WriteType_Native<f32>(value); }
		inline void WriteF64_LE(f64 value) { WriteType_Native<f64>(value); }

		inline void WriteI16_BE(i16 value) { WriteI16_LE(Utilities::ByteSwapI16(value)); }
		inline void WriteU16_BE(u16 value) { WriteU16_LE(Utilities::ByteSwapU16(value)); }
		inline void WriteI32_BE(i32 value) { WriteI32_LE(Utilities::ByteSwapI32(value)); }
		inline void WriteU32_BE(u32 value) { WriteU32_LE(Utilities::ByteSwapU32(value)); }
		inline void WriteI64_BE(i64 value) { WriteI64_LE(Utilities::ByteSwapI64(value)); }
		inline void WriteU64_BE(u64 value) { WriteU64_LE(Utilities::ByteSwapU64(value)); }
		inline void WriteF32_BE(f32 value) { WriteF32_LE(Utilities::ByteSwapF32(value)); }
		inline void WriteF64_BE(f64 value) { WriteF64_LE(Utilities::ByteSwapF64(value)); }

	protected:
		void OnPointerModeChanged() override;
		void OnEndiannessChanged() override;

	private:
		void(StreamWriter::*writePtrFunc)(FileAddr) = nullptr;
		void(StreamWriter::*writeSizeFunc)(size_t) = nullptr;
		void(StreamWriter::*writeI16Func)(i16) = nullptr;
		void(StreamWriter::*writeU16Func)(u16) = nullptr;
		void(StreamWriter::*writeI32Func)(i32) = nullptr;
		void(StreamWriter::*writeU32Func)(u32) = nullptr;
		void(StreamWriter::*writeI64Func)(i64) = nullptr;
		void(StreamWriter::*writeU64Func)(u64) = nullptr;
		void(StreamWriter::*writeF32Func)(f32) = nullptr;
		void(StreamWriter::*writeF64Func)(f64) = nullptr;

		struct Settings
		{
			bool PoolStrings = true;
		} settings;

		struct StringPointerEntry
		{
			FileAddr ReturnAddress;
			std::string_view String;
			i32 Alignment;
		};

		struct DelayedWriteEntry
		{
			FileAddr ReturnAddress;
			const std::function<void(StreamWriter&)> Function;
		};

		struct FunctionPointerEntry
		{
			FileAddr ReturnAddress;
			FileAddr BaseAddress;
			const std::function<void(StreamWriter&)> Function;
		};

		std::unordered_map<std::string, FileAddr> writtenStringPool;
		std::vector<StringPointerEntry> stringPointerPool;

		std::vector<DelayedWriteEntry> delayedWritePool;

		// NOTE: Using std::list to avoid invalidating previous entries while executing recursive pointer writes
		std::list<FunctionPointerEntry> pointerPool;
	};
}
