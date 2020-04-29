#include "Crypto.h"
#include "Detail/Win32Crypto.h"

namespace Comfy::IO::Crypto
{
	bool DecryptAesEcb(const u8* encryptedData, u8* decryptedData, size_t dataSize, const std::array<u8, KeySize>& key)
	{
		return Detail::Win32DecryptAesEcb(encryptedData, decryptedData, dataSize, key);
	}

	bool DecryptAesCbc(const u8* encryptedData, u8* decryptedData, size_t dataSize, const std::array<u8, KeySize>& key, const std::array<u8, IVSize>& iv)
	{
		return Detail::Win32DecryptAesCbc(encryptedData, decryptedData, dataSize, key, iv);
	}
}
