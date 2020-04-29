#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::IO::Crypto
{
	static constexpr size_t IVSize = 16;
	static constexpr size_t KeySize = 16;

	bool Win32DecryptAesEcb(const u8* encryptedData, u8* decryptedData, size_t dataSize, std::array<u8, KeySize> key);
	bool Win32DecryptAesCbc(const u8* encryptedData, u8* decryptedData, size_t dataSize, std::array<u8, KeySize> key, std::array<u8, IVSize> iv);
}
