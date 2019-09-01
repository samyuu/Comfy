#pragma once
#include "Types.h"
#include "TxpSet.h"

namespace FileSystem
{
	enum class GraphicsMode : uint32_t
	{
		QVGA = 0,
		VGA = 1,
		SVGA = 2,
		XGA = 3,
		UXGA = 6,
		WVGA = 7,
		WXGA = 9,
		WUXGA = 11,
		WQXGA = 12,
		HDTV720 = 13,
		HDTV1080 = 14,
		Custom = 18,
	};

	struct Sprite
	{
		int32_t TextureIndex;
		float Unknown;
		vec4 TexelRegion;
		vec4 PixelRegion;
		String Name;
		unk32_t GraphicsReserved;
		GraphicsMode GraphicsMode;
	};

	class SprSet : public IBinaryReadable, public IBufferParsable
	{
	public:
		String Name;
		uint32_t Signature;
		UniquePtr<TxpSet> TxpSet;
		Vector<Sprite> Sprites;

		virtual void Read(BinaryReader& reader) override;
		virtual void Parse(const uint8_t* buffer) override;

	private:
	};
}