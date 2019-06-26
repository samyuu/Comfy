#pragma once
#include "FileSystem/FileInterface.h"
#include <memory>
#include <string>
#include <vector>

class Texture2D;

namespace FileSystem
{
	enum TxpType : uint8_t
	{
		TxpType_MipMap = 2,
		TxpType_TxpSet = 3,
		TxpType_Texture = 4,
		TxpType_TextureAlt = 5,
	};

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
		TxpType Type;
	};

	struct MipMap
	{
		TxpSig Signature;
		int32_t Width;
		int32_t Height;
		TextureFormat Format;
		int32_t Index;
		std::vector<uint8_t> Data;
		struct
		{
			uint8_t* DataPointer;
			uint32_t DataPointerSize;
		};
	};

	struct Texture
	{
		TxpSig Signature;
		std::vector<std::shared_ptr<MipMap>> MipMaps;
		std::string Name;

		std::shared_ptr<Texture2D> Texture2D;
	};

	class TxpSet : public IBinaryReadable, IBufferParsable
	{
	public:
		TxpSig Signature;
		std::vector<std::shared_ptr<Texture>> Textures;

		virtual void Read(BinaryReader& reader) override;
		virtual void Parse(uint8_t* buffer) override;

	private:
	};
}
