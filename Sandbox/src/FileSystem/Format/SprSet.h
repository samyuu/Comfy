#pragma once
#include "Types.h"
#include "TxpSet.h"
#include <string>

namespace FileSystem
{
	struct Sprite
	{
		int32_t TextureIndex;
		float Unknown;
		vec4 TexelRegion;
		vec4 PixelRegion;
		std::string Name;
		struct
		{
			uint32_t Zero;
			uint32_t Low;
		} ExtraData;
	};

	class SprSet : public IBinaryReadable, IBufferParsable
	{
	public:
		std::string Name;
		uint32_t Signature;
		std::unique_ptr<TxpSet> TxpSet;
		std::vector<Sprite> Sprites;

		virtual void Read(BinaryReader& reader) override;
		virtual void Parse(uint8_t* buffer) override;

	private:
	};
}