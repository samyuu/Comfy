#pragma once
#include "Types.h"

namespace Comfy::IO
{
	enum class PtrMode : u8
	{
		Mode32Bit,
		Mode64Bit,
	};

	enum class Endianness : u8
	{
		Little,
		Big,

		// TODO: C++20 std::endian please come to rescue :PeepoHug:
		Native = (true) ? Little : Big,
	};
}
