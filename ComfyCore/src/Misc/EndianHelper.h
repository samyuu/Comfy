#pragma once
#include "Types.h"
#include <intrin.h>

namespace Utilities
{
	inline int16_t ByteswapInt16(int16_t value) { return _byteswap_ushort(value); };
	inline uint16_t ByteswapUInt16(uint16_t value) { return _byteswap_ushort(value); };

	inline int32_t ByteswapInt32(int32_t value) { return _byteswap_ulong(value); };
	inline uint32_t ByteswapUInt32(uint32_t value) { return _byteswap_ulong(value); };

	inline int64_t ByteswapInt64(int64_t value) { return _byteswap_uint64(value); };
	inline uint64_t ByteswapUInt64(uint64_t value) { return _byteswap_uint64(value); };

	inline float ByteswapFloat(float value) { uint32_t result = ByteswapUInt32(*reinterpret_cast<uint32_t*>(&value)); return *reinterpret_cast<float*>(&result); };
	inline double ByteswapDouble(double value) { uint64_t result = ByteswapUInt64(*reinterpret_cast<uint64_t*>(&value)); return *reinterpret_cast<double*>(&result); };
}
