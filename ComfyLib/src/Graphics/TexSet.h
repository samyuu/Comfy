#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "GPUResource.h"
#include "Resource/IDTypes.h"
#include "Graphics/GraphicTypes.h"
#include "IO/Stream/FileInterfaces.h"
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
		std::unique_ptr<u8[]> Data;
	};

	class Tex
	{
	public:
		std::optional<std::string> Name;

		// NOTE: Two dimensional array [CubeFace][MipMap]
		std::vector<std::vector<TexMipMap>> MipMapsArray;

		Cached_TexID ID = TexID::Invalid;

		mutable std::unique_ptr<GPUResource> GPU_Texture2D = nullptr;
		mutable std::unique_ptr<GPUResource> GPU_CubeMap = nullptr;

	public:
		const std::vector<TexMipMap>& GetMipMaps(u32 arrayIndex = 0) const;

		TxpSig GetSignature() const;
		ivec2 GetSize() const;
		TextureFormat GetFormat() const;

		static constexpr std::string_view UnknownName = "F_COMFY_UNKNOWN";
		std::string_view GetName() const;
	};

	class TexSet : public IO::IStreamWritable, public IO::IBufferParsable, NonCopyable
	{
	public:
		std::vector<std::shared_ptr<Tex>> Textures;

		void Parse(const u8* buffer, size_t bufferSize) override;
		void Write(IO::StreamWriter& writer) override;

		void SetTextureIDs(const class ObjSet& objSet);

	public:
		// TODO: Rename and or refactor
		static std::unique_ptr<TexSet> LoadSetTextureIDs(std::string_view filePath, const class ObjSet* objSet);

	private:
		void ParseTex(const u8* buffer, Tex& tex);
	};
}
