#pragma once
#include "Types.h"
#include "Graphics/TexSet.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Graphics
{
	struct Spr
	{
		i32 TextureIndex;
		i32 Rotate;
		vec4 TexelRegion;
		vec4 PixelRegion;
		std::string Name;
		struct ExtraData
		{
			u32 Flags;
			ScreenMode ScreenMode;
		} Extra;

		vec2 GetSize() const;
	};

	class SprSet : public IO::IBinaryWritable, public IO::IBufferParsable, NonCopyable
	{
	public:
		std::string Name;
		u32 Flags;
		UniquePtr<TexSet> TexSet;
		std::vector<Spr> Sprites;

		void Write(IO::BinaryWriter& writer) override;

		void Parse(const u8* buffer, size_t bufferSize) override;

	private:
	};
}
