#include "StreamWriter.h"

namespace Comfy::IO
{
	void StreamWriter::WriteStr(std::string_view value)
	{
		// NOTE: Manually write null terminator
		WriteBuffer(value.data(), value.size());
		WriteChar('\0');
	}

	void StreamWriter::WriteStrPtr(std::string_view value, i32 alignment)
	{
		stringPointerPool.push_back({ GetPosition(), value, alignment });
		WritePtr(FileAddr::NullPtr);
	}

	void StreamWriter::WriteFuncPtr(const std::function<void(StreamWriter&)>& func, FileAddr baseAddress)
	{
		pointerPool.push_back({ GetPosition(), baseAddress, func });
		WritePtr(FileAddr::NullPtr);
	}

	void StreamWriter::WriteDelayedPtr(const std::function<void(StreamWriter&)>& func)
	{
		delayedWritePool.push_back({ GetPosition(), func });
		WritePtr(FileAddr::NullPtr);
	}

	void StreamWriter::WritePadding(size_t size, u32 paddingValue)
	{
		if (size < 0)
			return;

		constexpr size_t maxSize = 32;
		assert(size <= maxSize);
		std::array<u8, maxSize> paddingValues;

		std::memset(paddingValues.data(), paddingValue, size);
		WriteBuffer(paddingValues.data(), size);
	}

	void StreamWriter::WriteAlignmentPadding(i32 alignment, u32 paddingValue)
	{
		constexpr bool forceExtraPadding = false;
		constexpr i32 maxAlignment = 32;

		assert(alignment <= maxAlignment);
		std::array<u8, maxAlignment> paddingValues;

		const i32 value = static_cast<i32>(GetPosition());
		i32 paddingSize = ((value + (alignment - 1)) & ~(alignment - 1)) - value;

		if (forceExtraPadding && paddingSize <= 0)
			paddingSize = alignment;

		if (paddingSize > 0)
		{
			std::memset(paddingValues.data(), paddingValue, paddingSize);
			WriteBuffer(paddingValues.data(), paddingSize);
		}
	}

	void StreamWriter::FlushStringPointerPool()
	{
		for (const auto& value : stringPointerPool)
		{
			const auto originalStringOffset = GetPosition();
			auto stringOffset = originalStringOffset;

			bool pooledStringFound = false;
			if (settings.PoolStrings)
			{
				if (auto pooledString = writtenStringPool.find(std::string(value.String)); pooledString != writtenStringPool.end())
				{
					stringOffset = pooledString->second;
					pooledStringFound = true;
				}
			}

			SetPosition(value.ReturnAddress);
			WritePtr(stringOffset);

			if (!pooledStringFound)
			{
				SetPosition(stringOffset);
				WriteStr(value.String);

				if (value.Alignment > 0)
					WriteAlignmentPadding(value.Alignment);
			}
			else
			{
				SetPosition(originalStringOffset);
			}

			if (settings.PoolStrings && !pooledStringFound)
				writtenStringPool.insert(std::make_pair(value.String, stringOffset));
		}

		if (settings.PoolStrings)
			writtenStringPool.clear();

		stringPointerPool.clear();
	}

	void StreamWriter::FlushPointerPool()
	{
		for (const auto& value : pointerPool)
		{
			const auto offset = GetPosition();

			SetPosition(value.ReturnAddress);
			WritePtr(offset - value.BaseAddress);

			SetPosition(offset);
			value.Function(*this);
		}

		pointerPool.clear();
	}

	void StreamWriter::FlushDelayedWritePool()
	{
		for (const auto& value : delayedWritePool)
		{
			const auto offset = GetPosition();

			SetPosition(value.ReturnAddress);
			value.Function(*this);

			SetPosition(offset);
		}

		delayedWritePool.clear();
	}

	void StreamWriter::OnPointerModeChanged()
	{
		switch (pointerMode)
		{
		case PtrMode::Mode32Bit:
			writePtrFunc = &StreamWriter::WritePtr_32;
			writeSizeFunc = &StreamWriter::WriteSize_32;
			return;

		case PtrMode::Mode64Bit:
			writePtrFunc = &StreamWriter::WritePtr_64;
			writeSizeFunc = &StreamWriter::WriteSize_64;
			return;

		default:
			assert(false);
			return;
		}
	}

	void StreamWriter::OnEndiannessChanged()
	{
		switch (endianness)
		{
		case Endianness::Little:
			writeI16Func = &StreamWriter::WriteI16_LE;
			writeU16Func = &StreamWriter::WriteU16_LE;
			writeI32Func = &StreamWriter::WriteI32_LE;
			writeU32Func = &StreamWriter::WriteU32_LE;
			writeI64Func = &StreamWriter::WriteI64_LE;
			writeU64Func = &StreamWriter::WriteU64_LE;
			writeF32Func = &StreamWriter::WriteF32_LE;
			writeF64Func = &StreamWriter::WriteF64_LE;
			return;

		case Endianness::Big:
			writeI16Func = &StreamWriter::WriteI16_BE;
			writeU16Func = &StreamWriter::WriteU16_BE;
			writeI32Func = &StreamWriter::WriteI32_BE;
			writeU32Func = &StreamWriter::WriteU32_BE;
			writeI64Func = &StreamWriter::WriteI64_BE;
			writeU64Func = &StreamWriter::WriteU64_BE;
			writeF32Func = &StreamWriter::WriteF32_BE;
			writeF64Func = &StreamWriter::WriteF64_BE;
			return;

		default:
			assert(false);
			return;
		}
	}
}
