#include "StreamReader.h"
#include <assert.h>

namespace Comfy::IO
{
	std::string StreamReader::ReadStr()
	{
		// NOTE: Account for the ending null byte
		size_t length = sizeof('\0');

		const auto prePos = GetPosition();
		{
			while (ReadChar() != '\0' && !EndOfFile())
				length++;
		}
		SetPosition(prePos);

		if (length == sizeof(char))
			return "";

		auto result = std::string(length - sizeof(char), '\0');
		ReadBuffer(result.data(), length * sizeof(char) - 1);
		SetPosition(GetPosition() + static_cast<FileAddr>(sizeof(char)));
		return result;
	}

	std::string StreamReader::ReadStrAt(FileAddr position)
	{
		return ReadAt<std::string>(position, [this](StreamReader&)
		{
			return ReadStr();
		});
	}

	std::string StreamReader::ReadStr(size_t size)
	{
		auto result = std::string(size, '\0');
		ReadBuffer(result.data(), size * sizeof(char));
		return result;
	}

	void StreamReader::OnPointerModeChanged()
	{
		switch (pointerMode)
		{
		case PtrMode::Mode32Bit:
			readPtrFunc = &ReadPtr32;
			readSizeFunc = &ReadSize32;
			return;

		case PtrMode::Mode64Bit:
			readPtrFunc = &ReadPtr64;
			readSizeFunc = &ReadSize64;
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
			readI16Func = &LE_ReadI16;
			readU16Func = &LE_ReadU16;
			readI32Func = &LE_ReadI32;
			readU32Func = &LE_ReadU32;
			readI64Func = &LE_ReadI64;
			readU64Func = &LE_ReadU64;
			readF32Func = &LE_ReadF32;
			readF64Func = &LE_ReadF64;
			return;

		case Endianness::Big:
			readI16Func = &BE_ReadI16;
			readU16Func = &BE_ReadU16;
			readI32Func = &BE_ReadI32;
			readU32Func = &BE_ReadU32;
			readI64Func = &BE_ReadI64;
			readU64Func = &BE_ReadU64;
			readF32Func = &BE_ReadF32;
			readF64Func = &BE_ReadF64;
			return;

		default:
			assert(false);
			return;
		}
	}
}
