#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"

namespace Comfy::Graphics::Utilities
{
	using SpriteMarkupFlags = uint32_t;
	enum SpriteMarkupFlagsEnum : SpriteMarkupFlags
	{
		SpriteMarkupFlags_None = 0,
		SpriteMarkupFlags_NoMerge = (1 << 0),
	};

	struct SpriteMarkup
	{
		std::string Name;
		ivec2 Size;
		const void* RGBAPixels;
		ScreenMode ScreenMode;
		SpriteMarkupFlags Flags;
	};

	class SpriteCreator : NonCopyable
	{
	public:
		SpriteCreator() = default;
		~SpriteCreator() = default;

	public:
		UniquePtr<SprSet> Create(const std::vector<SpriteMarkup>& spriteMarkups);

	protected:
		Spr CreateSprFromMarkup(const SpriteMarkup& markup);
		
		RefPtr<Tex> CreateNoMergeTex(const Spr& spr, const SpriteMarkup& markup, size_t index);

		const char* GetCompressionName(TextureFormat format) const;
		std::string FormatTextureName(bool merge, TextureFormat format, size_t index) const;

		ivec4 GetTexelRegionFromPixelRegion(const vec4& pixelRegion, vec2 textureSize);
	};
}
