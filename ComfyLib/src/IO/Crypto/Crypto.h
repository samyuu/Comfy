#pragma once
#include "Types.h"

namespace Comfy::IO::Crypto
{
	static constexpr size_t IVSize = 16;
	static constexpr size_t KeySize = 16;

	enum class BlockCipherMode : u32
	{
		CBC, ECB,
	};

	bool DecryptAesEcb(const u8* encryptedData, u8* decryptedData, size_t dataSize, const std::array<u8, KeySize>& key);
	bool DecryptAesCbc(const u8* encryptedData, u8* decryptedData, size_t dataSize, const std::array<u8, KeySize>& key, const std::array<u8, IVSize>& iv);
}
