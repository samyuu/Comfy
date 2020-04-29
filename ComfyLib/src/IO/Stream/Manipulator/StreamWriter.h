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

		template <typename T>
		void WriteType(T value) { WriteBuffer(&value, sizeof(T)); }

		void WriteStr(std::string_view value);
		void WriteStrPtr(std::string_view value, i32 alignment = 0);

		inline void WritePtr(FileAddr value) { writePtrFunc(*this, value); }
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
		inline void WriteF32(f32 value) { return WriteType<f32>(value); }
		inline void WriteF64(f64 value) { return WriteType<f64>(value); }

	protected:
		void OnPointerModeChanged() override;
		void OnEndiannessChanged() override;

	private:
		// TODO: LE/BE and Size-32/64
		using WritePtrFunc = void(StreamWriter&, FileAddr);

		WritePtrFunc* writePtrFunc = nullptr;

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

	private:
		static void WritePtr32(StreamWriter& writer, FileAddr value) { writer.WriteI32(static_cast<i32>(value)); }
		static void WritePtr64(StreamWriter& writer, FileAddr value) { writer.WriteI64(static_cast<i64>(value)); }
	};
}
