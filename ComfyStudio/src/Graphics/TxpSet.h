#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	struct TxpSig
	{
		enum TxpType : uint8_t
		{
			MipMap = 2,
			TxpSet = 3,
			Texture = 4,
			TextureAlt = 5,
		};

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
			const uint8_t* DataPointer;
			uint32_t DataPointerSize;
		};
	};

	struct Txp
	{
		TxpSig Signature;
		std::vector<RefPtr<MipMap>> MipMaps;
		std::string Name;

		RefPtr<class GL_Texture2D> GraphicsTexture;
	};

	class TxpSet : public FileSystem::IBinaryReadable, public FileSystem::IBufferParsable
	{
	public:
		TxpSig Signature;
		std::vector<RefPtr<Txp>> Textures;

		virtual void Read(FileSystem::BinaryReader& reader) override;
		virtual void Parse(const uint8_t* buffer) override;

		void UploadAll();

	private:
	};
}
