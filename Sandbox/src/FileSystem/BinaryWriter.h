#pragma once
#include "Stream.h"
#include "PtrMode.h"
#include <vector>
#include <list>
#include <functional>

namespace FileSystem
{
	constexpr uint32_t PaddingValue = 0xCCCCCCCC;

	class BinaryWriter
	{
	public:
		BinaryWriter();
		BinaryWriter(Stream* stream);
		~BinaryWriter();

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

		inline int64_t Write(const void* buffer, size_t size);

		template <typename T> void Write(T value) { Write(&value, sizeof(value)); };

		void WriteStr(const std::string& value);
		void WriteStrPtr(const std::string* value);

		inline void WritePtr(void* value) { writePtrFunction(this, value); };

		void WritePtr(const std::function<void(BinaryWriter&)>& func);
		void WriteDelayedPtr(const std::function<void(BinaryWriter&)>& func);

		void WriteAlignmentPadding(int32_t alignment, uint32_t paddingValue = PaddingValue);

		void FlushStringPointerPool();
		void FlushPointerPool();
		void FlushDelayedWritePool();

		inline void WriteBool(bool value) { return Write<bool>(value); };
		inline void WriteChar(char value) { return Write<char>(value); };
		inline void WriteInt8(uint8_t value) { return Write<int8_t>(value); };
		inline void WriteUInt8(uint8_t value) { return Write<uint8_t>(value); };
		inline void WriteInt16(int16_t value) { return Write<int16_t>(value); };
		inline void WriteUInt16(uint16_t value) { return Write<uint16_t>(value); };
		inline void WriteInt32(int32_t value) { return Write<int32_t>(value); };
		inline void WriteUInt32(uint32_t value) { return Write<uint32_t>(value); };
		inline void WriteInt64(int64_t value) { return Write<int64_t>(value); };
		inline void WriteUInt64(uint64_t value) { return Write<uint64_t>(value); };
		inline void WriteFloat(float value) { return Write<float>(value); };
		inline void WriteDouble(double value) { return Write<double>(value); };

	protected:
		typedef void (*WritePtr_t)(BinaryWriter*, void*);
		WritePtr_t writePtrFunction = nullptr;

		PtrMode pointerMode;
		bool leaveOpen = false;
		Stream* stream = nullptr;

		struct StringPointerEntry
		{
			void* ReturnAddress;
			const std::string* String;
		};

		struct FunctionPointerEntry
		{
			void* ReturnAddress;
			const std::function<void(BinaryWriter&)> Function;
		};

		struct DelayedWriteEntry
		{
			void* ReturnAddress;
			const std::function<void(BinaryWriter&)> Function;
		};

		std::vector<StringPointerEntry> stringPointerPool;
		std::list<FunctionPointerEntry> pointerPool;
		std::vector<DelayedWriteEntry> delayedWritePool;

	private:
		static void Write32BitPtr(BinaryWriter* writer, void* value) { writer->WriteInt32(static_cast<int32_t>((ptrdiff_t)value)); };
		static void Write64BitPtr(BinaryWriter* writer, void* value) { writer->WriteInt64(static_cast<int64_t>((ptrdiff_t)value)); };
	};

}