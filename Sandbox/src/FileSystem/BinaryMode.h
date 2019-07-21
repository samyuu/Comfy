#pragma once
#include "Types.h"

namespace FileSystem
{
	enum class PtrMode : uint8_t
	{
		Mode32Bit,
		Mode64Bit,
	};

	enum class Endianness : uint16_t
	{
		Little = 'LE',
		Big = 'BE',
	};
}