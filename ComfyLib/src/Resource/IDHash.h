#pragma once
#include "IDTypes.h"

namespace Comfy
{
	constexpr u32 MurmurHashSeed = 0xDEADBEEF;
	constexpr u32 MurmurHashM = 0x7FD652AD;
	constexpr u32 MurmurHashR = 0x10;

	constexpr u32 MurmurHash(std::string_view string)
	{
		size_t dataSize = string.size();
		const char* data = string.data();

		u32 hash = MurmurHashSeed;
		while (dataSize >= sizeof(u32))
		{
			hash += static_cast<u32>(
				(static_cast<u8>(data[0]) << 0) |
				(static_cast<u8>(data[1]) << 8) |
				(static_cast<u8>(data[2]) << 16) |
				(static_cast<u8>(data[3]) << 24));

			hash *= MurmurHashM;
			hash ^= hash >> MurmurHashR;

			data += sizeof(u32);
			dataSize -= sizeof(u32);
		}

		switch (dataSize)
		{
		case 3:
			hash += static_cast<u32>(data[2] << 16);
		case 2:
			hash += static_cast<u32>(data[1] << 8);
		case 1:
			hash += static_cast<u32>(data[0] << 0);
			hash *= MurmurHashM;
			hash ^= hash >> MurmurHashR;
			break;
		}

		hash *= MurmurHashM;
		hash ^= hash >> 10;
		hash *= MurmurHashM;
		hash ^= hash >> 17;

		return hash;
	}

	template <typename IDType>
	constexpr IDType HashIDString(std::string_view string)
	{
		return static_cast<IDType>(MurmurHash(string));
	}
}
