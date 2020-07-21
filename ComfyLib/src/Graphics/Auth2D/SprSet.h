#pragma once
#include "Types.h"
#include "Graphics/TexSet.h"
#include "Graphics/GraphicTypes.h"
#include "Database/SprDB.h"

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

	class SprSet : public IO::IStreamReadable, public IO::IStreamWritable, NonCopyable
	{
	public:
		std::string Name;
		u32 Flags;
		TexSet TexSet;
		std::vector<Spr> Sprites;

	public:
		void ApplyDBNames(const Database::SprSetEntry& sprSetEntry);

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
	};
}
