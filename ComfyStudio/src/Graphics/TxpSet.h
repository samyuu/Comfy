#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Resource/IDTypes.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/Direct3D/Texture/D3D_Texture.h"

namespace Comfy::Graphics
{
	struct TxpSig
	{
		enum TxpType : uint8_t
		{
			MipMap = 2,
			TxpSet = 3,
			Texture2D = 4,
			CubeMap = 5,
			Rectangle = 6,
		};

		std::array<char, 3> Txp;
		TxpType Type;
	};

	struct MipMap
	{
		TxpSig Signature;
		ivec2 Size;
		TextureFormat Format;
		uint8_t MipIndex;
		uint8_t ArrayIndex;

		const uint8_t* DataPointer;
		uint32_t DataPointerSize;
	};

	struct Txp
	{
		TxpSig Signature;
		std::string Name;

		uint8_t MipLevels;
		uint8_t ArraySize;

		// NOTE: Two dimensional array [CubeFace][MipMap]
		std::vector<std::vector<MipMap>> MipMapsArray;

		Cached_TxpID ID = TxpID::Invalid;

		UniquePtr<D3D_Texture2D> D3D_Texture2D = nullptr;
		UniquePtr<D3D_CubeMap> D3D_CubeMap = nullptr;

	public:
		const std::vector<MipMap>& GetMipMaps(uint32_t arrayIndex = 0) const;

		ivec2 GetSize() const;
		TextureFormat GetFormat() const;
	};

	class TxpSet : public FileSystem::IBufferParsable
	{
	public:
		TxpSig Signature;
		std::vector<RefPtr<Txp>> Txps;
		
		void Parse(const uint8_t* buffer, size_t bufferSize) override;
		void UploadAll(class SprSet* parentSprSet);

		void SetTextureIDs(const class ObjSet& objSet);

	public:
		static UniquePtr<TxpSet> MakeUniqueReadParseUpload(std::string_view filePath, const class ObjSet* objSet);

	private:
		void ParseTxp(const uint8_t* buffer, Txp& txp);
	};
}
