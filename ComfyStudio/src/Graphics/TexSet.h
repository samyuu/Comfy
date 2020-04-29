#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Resource/IDTypes.h"
#include "IO/FileInterface.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/GPU/GPUResources.h"
#include <optional>

namespace Comfy::Graphics
{
	enum class TxpSig : u32
	{
		MipMap = '\02PXT',
		TexSet = '\03PXT',
		Texture2D = '\04PXT',
		CubeMap = '\05PXT',
		Rectangle = '\06PXT',
	};

	struct TexMipMap
	{
		ivec2 Size;
		TextureFormat Format;

		u32 DataSize;
		UniquePtr<u8[]> Data;
	};

	struct Tex
	{
		std::optional<std::string> Name;

		// NOTE: Two dimensional array [CubeFace][MipMap]
		std::vector<std::vector<TexMipMap>> MipMapsArray;

		Cached_TexID ID = TexID::Invalid;

		UniquePtr<GPU_Texture2D> GPU_Texture2D = nullptr;
		UniquePtr<GPU_CubeMap> GPU_CubeMap = nullptr;

	public:
		const std::vector<TexMipMap>& GetMipMaps(u32 arrayIndex = 0) const;

		TxpSig GetSignature() const;
		ivec2 GetSize() const;
		TextureFormat GetFormat() const;

		static constexpr std::string_view UnknownName = "F_COMFY_UNKNOWN";
		std::string_view GetName() const;
	};

	class TexSet : public IO::IBinaryWritable, public IO::IBufferParsable, NonCopyable
	{
	public:
		std::vector<RefPtr<Tex>> Textures;

		void Write(IO::StreamWriter& writer) override;

		void Parse(const u8* buffer, size_t bufferSize) override;
		void UploadAll(class SprSet* parentSprSet);

		void SetTextureIDs(const class ObjSet& objSet);

	public:
		static UniquePtr<TexSet> MakeUniqueReadParseUpload(std::string_view filePath, const class ObjSet* objSet);

	private:
		void ParseTex(const u8* buffer, Tex& tex);
	};
}
