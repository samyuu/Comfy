#pragma once
#include "Types.h"
#include "IO/Stream/FileInterfaces.h"
#include "Graphics/Auth2D/SprSet.h"

namespace Comfy::Graphics
{
	struct FontGlyph
	{
		char16_t Character;
		bool IsNarrow;
		u8 Padding;
		u8 Row;
		u8 Column;
		u8 SizeX;
		u8 SizeY;
	};

	class FontMap;

	class BitmapFont
	{
		friend class FontMap;

	public:
		BitmapFont() = default;
		BitmapFont(const BitmapFont& other) = delete;
		BitmapFont(BitmapFont&& other);
		~BitmapFont() = default;

	public:
		// NOTE: Stored here as a mere convinience to directly access while rendering
		std::shared_ptr<Tex> Texture = nullptr;
		vec4 SpritePixelRegion = vec4(0.0f, 0.0f, 0.0f, 0.0f);

	public:
		// NOTE: Pixel size of the font
		ivec2 GetFontSize() const;

		// NOTE: Pixel size of a glyph source region including the usually 2 pixel padding
		ivec2 GetGlyphSize() const;

		// NOTE: Parameter char32_t for a cleaner and more extensible API however any character larger than char16_t will be invalid
		const FontGlyph* GetGlyph(char32_t character) const;

	private:
		struct DescriptionData
		{
			u32 TextureIndex;
			ivec2 FontSize;
			ivec2 GlyphSize;
			u16 Unknown0;
			u16 Unknown1;
			u32 Unknown2;
			u32 Unknown3;
			u8 Unknown4;
			u8 Unknown5;
			u8 Unknown6;
			u8 Unknown7;
			u32 Unknown8;
		} description = {};

		std::vector<FontGlyph> glyphs;

		struct CharacterToGlyphIndexLookupData
		{
			size_t TableSize;
			std::unique_ptr<u32[]> Table;
		} lookup = {};
	};

	class FontMap : public IO::IStreamReadable, NonCopyable
	{
	public:
		FontMap() = default;
		~FontMap() = default;

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;

		BitmapFont* FindFont(ivec2 fontSize);
		const BitmapFont* FindFont(ivec2 fontSize) const;

	public:
		std::vector<BitmapFont> Fonts;
	};
}
