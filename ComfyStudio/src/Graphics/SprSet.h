#pragma once
#include "Types.h"
#include "TxpSet.h"
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	struct Spr
	{
		int32_t TextureIndex;
		float Unknown;
		vec4 TexelRegion;
		vec4 PixelRegion;
		std::string Name;
		unk32_t GraphicsReserved;
		DisplayMode DisplayMode;

		inline vec2 GetSize() const { return vec2(PixelRegion.z, PixelRegion.w); };
	};

	class SprSet : public FileSystem::IBinaryReadable, public FileSystem::IBufferParsable
	{
	public:
		std::string Name;
		uint32_t Signature;
		UniquePtr<TxpSet> TxpSet;
		std::vector<Spr> Sprites;

		virtual void Read(FileSystem::BinaryReader& reader) override;
		virtual void Parse(const uint8_t* buffer) override;

	private:
	};
}