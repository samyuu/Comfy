#include "FontMap.h"
#include "IO/Stream/Manipulator/StreamReader.h"

using namespace Comfy::IO;

namespace Comfy::Graphics
{
	ivec2 BitmapFont::GetFontSize() const
	{
		return description.FontSize;
	}

	ivec2 BitmapFont::GetGlyphSize() const
	{
		return description.GlyphSize;
	}

	const FontGlyph* BitmapFont::GetGlyph(char32_t character) const
	{
		// NOTE: This would mean it's being accessed without having been parsed or after having been moved from
		assert(lookup.Table != nullptr);

		if (character > std::numeric_limits<char16_t>::max())
			return nullptr;

		if (character >= lookup.TableSize)
			return nullptr;

		const auto glyphIndex = lookup.Table[character];
		return (glyphIndex < glyphs.size()) ? &glyphs[glyphIndex] : nullptr;
	}

	BitmapFont::BitmapFont(BitmapFont&& other)
	{
		description = other.description;
		glyphs = std::move(other.glyphs);
		lookup.TableSize = other.lookup.TableSize;
		lookup.Table = std::move(other.lookup.Table);
	}

	void FontMap::Read(IO::StreamReader& reader)
	{
		const u32 magic = reader.ReadU32();
		if (magic != Util::ByteSwapU32('FMH3'))
			return;

		const u32 reserved = reader.ReadU32();
		const size_t fontCount = reader.ReadSize();
		const FileAddr fontsPtr = reader.ReadPtr();

		if (fontCount < 1 || fontsPtr == FileAddr::NullPtr)
			return;

		Fonts.resize(fontCount);
		reader.ReadAt(fontsPtr, [&](IO::StreamReader& reader)
		{
			for (auto& font : Fonts)
			{
				const FileAddr fontPtr = reader.ReadPtr();
				if (fontPtr == FileAddr::NullPtr)
					continue;

				reader.ReadAt(fontPtr, [&](IO::StreamReader& reader)
				{
					font.description.TextureIndex = reader.ReadU32();
					font.description.FontSize.x = reader.ReadU8();
					font.description.FontSize.y = reader.ReadU8();
					font.description.GlyphSize.x = reader.ReadU8();
					font.description.GlyphSize.y = reader.ReadU8();
					font.description.Unknown0 = reader.ReadU16();
					font.description.Unknown1 = reader.ReadU16();
					font.description.Unknown2 = reader.ReadU32();
					font.description.Unknown3 = reader.ReadU32();
					const u32 glyphCount = reader.ReadU32();
					font.description.Unknown4 = reader.ReadU8();
					font.description.Unknown5 = reader.ReadU8();
					font.description.Unknown6 = reader.ReadU8();
					font.description.Unknown7 = reader.ReadU8();
					font.description.Unknown8 = reader.ReadU32();

					char16_t largestCharacter = {};
					font.glyphs.resize(glyphCount);

					for (auto& glyph : font.glyphs)
					{
						glyph.Character = reader.ReadU16();
						glyph.IsNarrow = reader.ReadBool();
						glyph.Padding = reader.ReadU8();
						glyph.Row = reader.ReadU8();
						glyph.Column = reader.ReadU8();
						glyph.SizeX = reader.ReadU8();
						glyph.SizeY = reader.ReadU8();

						if (glyph.Character > largestCharacter)
							largestCharacter = glyph.Character;
					}

					// NOTE: This should be safe because FontGlyph::Character is only 16 bit
					//		 meaning storing a single continuous char16_t -> glyph index lookup table has a reasonably small memory footprint
					font.lookup.TableSize = (largestCharacter + 1);
					font.lookup.Table = std::make_unique<u32[]>(font.lookup.TableSize);

					for (size_t i = 0; i < font.lookup.TableSize; i++)
						font.lookup.Table[i] = std::numeric_limits<u32>::max();

					for (u32 i = 0; i < static_cast<u32>(font.glyphs.size()); i++)
						font.lookup.Table[font.glyphs[i].Character] = i;
				});
			}
		});
	}

	BitmapFont* FontMap::FindFont(ivec2 fontSize)
	{
		return FindIfOrNull(Fonts, [fontSize](auto& font) { return font.GetFontSize() == fontSize; });
	}

	const BitmapFont* FontMap::FindFont(ivec2 fontSize) const
	{
		return FindIfOrNull(Fonts, [fontSize](const auto& font) { return font.GetFontSize() == fontSize; });
	}
}
