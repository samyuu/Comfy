#include "IDHash.h"

namespace
{
	uint32_t MurmurHash(const uint8_t* data, size_t dataSize)
	{
		constexpr uint32_t m = 0x7FD652AD;
		constexpr uint32_t r = 0x10;

		uint32_t hash = 0xDEADBEEF;
		while (dataSize >= sizeof(uint32_t))
		{
			hash += *reinterpret_cast<const uint32_t*>(data);
			hash *= m;
			hash ^= hash >> r;

			data += sizeof(uint32_t);
			dataSize -= sizeof(uint32_t);
		}

		switch (dataSize)
		{
		case 3:
			hash += static_cast<uint32_t>(data[2] << 16);
		case 2:
			hash += static_cast<uint32_t>(data[1] << 8);
		case 1:
			hash += static_cast<uint32_t>(data[0] << 0);
			hash *= m;
			hash ^= hash >> r;
			break;
		}

		hash *= m;
		hash ^= hash >> 10;
		hash *= m;
		hash ^= hash >> 17;

		return hash;
	}
}

uint32_t HashIDString(std::string_view value)
{
	return MurmurHash(reinterpret_cast<const uint8_t*>(value.data()), value.length());
}
