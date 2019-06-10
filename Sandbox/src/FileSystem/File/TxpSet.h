#pragma once
#include <string>
#include <vector>
#include <memory>
#include "../FileInterface.h"

namespace File
{
	enum TextureFormat : uint32_t
	{
		TextureFormat_RGB = 1,
		TextureFormat_RGBA = 2,
		TextureFormat_RGBA4 = 5,
		TextureFormat_DXT1 = 6,
		TextureFormat_DXT3 = 7,
		TextureFormat_DXT4 = 8,
		TextureFormat_DXT5 = 9,
		TextureFormat_ATI1 = 10,
		TextureFormat_ATI2 = 11,
	};

	struct TxpSig
	{
		char Signature[3];
		uint8_t Type;
	};

	struct MipMap
	{
		TxpSig Signature;
		int32_t Width;
		int32_t Height;
		TextureFormat Format;
		int32_t Index;
		std::shared_ptr<std::vector<uint8_t>> Data;
	};

	struct Texture
	{
		TxpSig Signature;
		std::vector<std::shared_ptr<MipMap>> MipMaps;
		std::string Name;
	};

	class TxpSet : public IBinaryReadable
	{
	public:
		TxpSig Signature;
		std::vector<Texture> Textures;

		virtual void Read(BinaryReader& reader) override;

	private:
	};
}
