#include "SpriteCreator.h"

namespace Comfy::Graphics::Utilities
{
	UniquePtr<SprSet> SpriteCreator::Create(const std::vector<SpriteMarkup>& spriteMarkups)
	{
		auto result = MakeUnique<SprSet>();
		result->TexSet = MakeUnique<TexSet>();

		SprSet& sprSet = *result;
		TexSet& texSet = *sprSet.TexSet;

		sprSet.Flags = 0;

		sprSet.Sprites.reserve(spriteMarkups.size());
		for (const auto& markup : spriteMarkups)
			sprSet.Sprites.push_back(CreateSprFromMarkup(markup));

		constexpr bool noMerging = true;
		texSet.Textures.reserve(spriteMarkups.size());

		for (size_t i = 0; i < sprSet.Sprites.size(); i++)
		{
			auto& spr = sprSet.Sprites[i];
			auto& markup = spriteMarkups[i];

			spr.TextureIndex = static_cast<int32_t>(texSet.Textures.size());

			if ((markup.Flags & SpriteMarkupFlags_NoMerge) || noMerging)
			{
				const auto& tex = *texSet.Textures.emplace_back(CreateNoMergeTex(spr, markup, i));

				spr.PixelRegion = vec4(vec2(0.0f, 0.0f), vec2(tex.GetSize()));
				spr.TexelRegion = GetTexelRegionFromPixelRegion(spr.PixelRegion, tex.GetSize());
			}
			else
			{
				// TODO:
			}
		}

		return result;
	}

	Spr SpriteCreator::CreateSprFromMarkup(const SpriteMarkup& markup)
	{
		Spr result = {};
		result.TextureIndex = 0;
		result.Rotate = 0;
		result.Name = markup.Name;
		result.Extra.Flags = 0;
		result.Extra.ScreenMode = markup.ScreenMode;
		return result;
	}

	RefPtr<Tex> SpriteCreator::CreateNoMergeTex(const Spr& spr, const SpriteMarkup& markup, size_t index)
	{
		auto result = MakeRef<Tex>();

		result->Name = FormatTextureName(false, TextureFormat::RGBA8, index);

		auto& mipMaps = result->MipMapsArray.emplace_back();
		auto& baseMipMap = mipMaps.emplace_back();
		baseMipMap.Format = TextureFormat::RGBA8;
		baseMipMap.Size = markup.Size;
		baseMipMap.DataSize = (baseMipMap.Size.x * baseMipMap.Size.y * 4);
		baseMipMap.Data = MakeUnique<uint8_t[]>(baseMipMap.DataSize);
		std::memcpy(baseMipMap.Data.get(), markup.RGBAPixels, baseMipMap.DataSize);

		return result;
	}

	const char* SpriteCreator::GetCompressionName(TextureFormat format) const
	{
		switch (format)
		{
		case TextureFormat::A8:
		case TextureFormat::RGB8:
		case TextureFormat::RGBA8:
		case TextureFormat::RGB5:
		case TextureFormat::RGB5_A1:
		case TextureFormat::RGBA4:
		case TextureFormat::L8:
		case TextureFormat::L8A8:
			return "NOCOMP";
		case TextureFormat::DXT1:
		case TextureFormat::DXT1a:
			return "BC1COMP";
		case TextureFormat::DXT3:
			return "BC2COMP";
		case TextureFormat::DXT5:
			return "BC3COMP";
		case TextureFormat::RGTC1:
			return "BC4COMP";
		case TextureFormat::RGTC2:
			return "BC5COMP";
		default:
			return "UNKCOMP";
		}

		return nullptr;
	}

	std::string SpriteCreator::FormatTextureName(bool merge, TextureFormat format, size_t index) const
	{
		const char* mergeString = merge ? "MERGE" : "NOMERGE";
		const char* compressionString = GetCompressionName(format);

		char nameBuffer[32];
		sprintf_s(nameBuffer, "%s_%s_%03zu", mergeString, compressionString, index);
		return nameBuffer;
	}

	ivec4 SpriteCreator::GetTexelRegionFromPixelRegion(const vec4& pixelRegion, vec2 textureSize)
	{
		vec4 texelRegion;
		texelRegion.x = (pixelRegion.x / textureSize.x);
		texelRegion.y = (pixelRegion.y / textureSize.y);
		texelRegion.z = (pixelRegion.z / textureSize.x);
		texelRegion.w = (pixelRegion.w / textureSize.y);
		return texelRegion;
	}
}
