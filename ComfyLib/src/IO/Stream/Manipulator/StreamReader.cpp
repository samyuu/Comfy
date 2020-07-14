#include "StreamReader.h"

namespace Comfy::IO
{
	std::string StreamReader::ReadStr()
	{
		// NOTE: Account for the ending null byte
		size_t length = sizeof('\0');

		ReadAt(GetPosition(), [&length](StreamReader& reader) 
		{
			while (reader.ReadChar() != '\0' && !reader.EndOfFile())
				length++;
		});

		if (length == sizeof(char))
			return "";

		auto result = std::string(length - sizeof(char), '\0');
		ReadBuffer(result.data(), length * sizeof(char) - 1);
		Skip(static_cast<FileAddr>(sizeof(char)));
		return result;
	}

	std::string StreamReader::ReadStrAtOffsetAware(FileAddr position)
	{
		return ReadValueAtOffsetAware<std::string>(position, [](StreamReader& reader)
		{
			return reader.ReadStr();
		});
	}

	std::string StreamReader::ReadStr(size_t size)
	{
		auto result = std::string(size, '\0');
		ReadBuffer(result.data(), size * sizeof(char));
		return result;
	}

	std::string StreamReader::ReadStrPtrOffsetAware()
	{
		return ReadStrAtOffsetAware(ReadPtr());
	}

	void StreamReader::OnPointerModeChanged()
	{
		switch (pointerMode)
		{
		case PtrMode::Mode32Bit:
			readPtrFunc = &StreamReader::ReadPtr_32;
			readSizeFunc = &StreamReader::ReadSize_32;
			return;

		case PtrMode::Mode64Bit:
			readPtrFunc = &StreamReader::ReadPtr_64;
			readSizeFunc = &StreamReader::ReadSize_64;
			return;

		default:
			assert(false);
			return;
		}
	}

	void StreamReader::OnEndiannessChanged()
	{
		switch (endianness)
		{
		case Endianness::Little:
			readI16Func = &StreamReader::ReadI16_LE;
			readU16Func = &StreamReader::ReadU16_LE;
			readI32Func = &StreamReader::ReadI32_LE;
			readU32Func = &StreamReader::ReadU32_LE;
			readI64Func = &StreamReader::ReadI64_LE;
			readU64Func = &StreamReader::ReadU64_LE;
			readF32Func = &StreamReader::ReadF32_LE;
			readF64Func = &StreamReader::ReadF64_LE;
			return;

		case Endianness::Big:
			readI16Func = &StreamReader::ReadI16_BE;
			readU16Func = &StreamReader::ReadU16_BE;
			readI32Func = &StreamReader::ReadI32_BE;
			readU32Func = &StreamReader::ReadU32_BE;
			readI64Func = &StreamReader::ReadI64_BE;
			readU64Func = &StreamReader::ReadU64_BE;
			readF32Func = &StreamReader::ReadF32_BE;
			readF64Func = &StreamReader::ReadF64_BE;
			return;

		default:
			assert(false);
			return;
		}
	}
}
