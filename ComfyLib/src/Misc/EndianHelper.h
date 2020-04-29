#pragma once
#include "Types.h"
#include <intrin.h>

namespace Comfy::Utilities
{
	inline i16 ByteSwapI16(i16 value) { return _byteswap_ushort(value); }
	inline u16 ByteSwapU16(u16 value) { return _byteswap_ushort(value); }

	inline i32 ByteSwapI32(i32 value) { return _byteswap_ulong(value); }
	inline u32 ByteSwapU32(u32 value) { return _byteswap_ulong(value); }

	inline int64_t ByteSwapI64(int64_t value) { return _byteswap_uint64(value); }
	inline u64 ByteSwapU64(u64 value) { return _byteswap_uint64(value); }

	inline float ByteSwapF32(float value) { u32 result = ByteSwapU32(*reinterpret_cast<u32*>(&value)); return *reinterpret_cast<float*>(&result); }
	inline double ByteSwapF64(double value) { u64 result = ByteSwapU64(*reinterpret_cast<u64*>(&value)); return *reinterpret_cast<double*>(&result); }
}
