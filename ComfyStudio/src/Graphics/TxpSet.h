#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Resource/IDTypes.h"
#include "FileSystem/FileInterface.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/GPU/GPUResources.h"
#include <optional>

namespace Comfy::Graphics
{
	enum class TxpSig
	{
		MipMap = '\02PXT',
		TxpSet = '\03PXT',
		Texture2D = '\04PXT',
		CubeMap = '\05PXT',
		Rectangle = '\06PXT',
	};

	struct TxpMipMap
	{
		TxpSig Signature;
		ivec2 Size;
		TextureFormat Format;
		uint8_t MipIndex;
		uint8_t ArrayIndex;

		uint32_t DataSize;
		UniquePtr<uint8_t[]> Data;
	};

	struct Txp
	{
		TxpSig Signature;
		std::optional<std::string> Name;

		uint8_t MipLevels;
		uint8_t ArraySize;

		// NOTE: Two dimensional array [CubeFace][MipMap]
		std::vector<std::vector<TxpMipMap>> MipMapsArray;

		Cached_TxpID ID = TxpID::Invalid;

		UniquePtr<GPU_Texture2D> GPU_Texture2D = nullptr;
		UniquePtr<GPU_CubeMap> GPU_CubeMap = nullptr;

	public:
		const std::vector<TxpMipMap>& GetMipMaps(uint32_t arrayIndex = 0) const;

		ivec2 GetSize() const;
		TextureFormat GetFormat() const;

		static constexpr std::string_view UnknownName = "F_COMFY_UNKNOWN";
		std::string_view GetName() const;
	};

	class TxpSet : public FileSystem::IBufferParsable, NonCopyable
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
