#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "../Crypto.h"

namespace Comfy::IO::Crypto::Detail
{
	bool Win32DecryptAesEcb(const u8* encryptedData, u8* decryptedData, size_t dataSize, std::array<u8, KeySize> key);
	bool Win32DecryptAesCbc(const u8* encryptedData, u8* decryptedData, size_t dataSize, std::array<u8, KeySize> key, const std::array<u8, IVSize> iv);
}
