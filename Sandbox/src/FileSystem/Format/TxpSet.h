#pragma once
#include "FileSystem/FileInterface.h"
#include <memory>
#include <string>
#include <vector>

class Texture2D;

enum class TextureFormat : int32_t
{
	Unknown = -1,
	// GL_ALPHA8
	A8 = 0,
	// GL_RGB8
	RGB8 = 1,
	// GL_RGBA8
	RGBA8 = 2,
	// GL_RGB5
	RGB5 = 3,
	// GL_RGB5_A1
	RGB5_A1 = 4,
	// GL_RGBA4
	RGBA4 = 5,
	// GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	DXT1 = 6,
	// GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	DXT1a = 7,
	// GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	DXT3 = 8,
	// GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	DXT5 = 9,
	// GL_COMPRESSED_RED_RGTC1
	RGTC1 = 10,
	// GL_COMPRESSED_RG_RGTC2
	RGTC2 = 11,
	// GL_LUMINANCE8
	L8 = 12,
	// GL_LUMINANCE8_ALPHA8
	L8A8 = 13,
};

namespace FileSystem
{
	enum TxpType : uint8_t
	{
		TxpType_MipMap = 2,
		TxpType_TxpSet = 3,
		TxpType_Texture = 4,
		TxpType_TextureAlt = 5,
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
		void UploadTexture2D();
	};

	class TxpSet : public IBinaryReadable, IBufferParsable
	{
	public:
		TxpSig Signature;
		std::vector<std::shared_ptr<Texture>> Textures;

		virtual void Read(BinaryReader& reader) override;
		virtual void Parse(uint8_t* buffer) override;
		
		void UploadAll();

	private:
	};
}
