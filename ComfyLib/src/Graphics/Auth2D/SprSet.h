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

	class SprSet : public IO::IStreamWritable, public IO::IBufferParsable, NonCopyable
	{
	public:
		std::string Name;
		u32 Flags;
		std::unique_ptr<TexSet> TexSet;
		std::vector<Spr> Sprites;

		IO::StreamResult Write(IO::StreamWriter& writer) override;

		void Parse(const u8* buffer, size_t bufferSize) override;

	private:
	};
}
