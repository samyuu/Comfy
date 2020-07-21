#include "SpriteExtraction.h"
#include "TextureCompression.h"
#include "IO/Path.h"
#include "Misc/ImageHelper.h"
#include "Misc/StringUtil.h"
#include <future>

namespace Comfy::Graphics::Utilities
{
	namespace
	{
		constexpr size_t SpritePixelCount(const Spr& spr)
		{
			return static_cast<size_t>(spr.PixelRegion.z) * static_cast<size_t>(spr.PixelRegion.w);
		}

		size_t SumTotalSpritePixels(const SprSet& sprSet)
		{
			size_t totalPixels = 0;

			for (const auto& spr : sprSet.Sprites)
				totalPixels += SpritePixelCount(spr);

			return totalPixels;
		}

		constexpr bool SpriteFitsInTexture(ivec2 sprPos, ivec2 sprSize, ivec2 texSize)
		{
			return (sprPos.x >= 0 && sprPos.x + sprSize.x <= texSize.x && sprPos.y >= 0 && sprPos.y + sprSize.y <= texSize.y);
		}
	}

	void ExtractAllSprPNGs(std::string_view outputDirectory, const SprSet& sprSet)
	{
		if (sprSet.Sprites.empty() || sprSet.TexSet.Textures.empty())
			return;

		const auto& textures = sprSet.TexSet.Textures;
		const auto& sprites = sprSet.Sprites;

		const auto rgbaTextures = std::make_unique<std::unique_ptr<u8[]>[]>(textures.size());
		const auto textureFutures = std::make_unique<std::future<void>[]>(textures.size());

		for (size_t i = 0; i < textures.size(); i++)
		{
			textureFutures[i] = std::async(std::launch::async, [&, i]
			{
				auto& inTexture = *textures[i];
				auto& outRGBA = rgbaTextures[i];

				const auto rgbaByteSize = Utilities::TextureFormatByteSize(inTexture.GetSize(), TextureFormat::RGBA8);
				outRGBA = std::make_unique<u8[]>(rgbaByteSize);

				if (!Utilities::ConvertTextureToRGBABuffer(inTexture, outRGBA.get(), rgbaByteSize))
					outRGBA = nullptr;
			});
		}

		// NOTE: While the textures are still being decompressed in the background start allocating a large memory region to be and split up and used by each sprite
		const auto combinedSprPixelsBuffer = std::make_unique<u32[]>(SumTotalSpritePixels(sprSet));
		const auto spriteFutures = std::make_unique<std::future<void>[]>(sprites.size());

		for (size_t i = 0; i < textures.size(); i++)
			textureFutures[i].wait();

		for (size_t i = 0, pixelIndex = 0; i < sprites.size(); i++)
		{
			u32* sprRGBA = &combinedSprPixelsBuffer[pixelIndex];
			pixelIndex += SpritePixelCount(sprites[i]);

			spriteFutures[i] = std::async(std::launch::async, [&, sprIndex = i, sprRGBA]
			{
				const auto& spr = sprites[sprIndex];
				if (!InBounds(spr.TextureIndex, textures))
					return;

				const auto sprPos = ivec2(spr.PixelRegion.x, spr.PixelRegion.y);
				const auto sprSize = ivec2(spr.GetSize());
				const auto texSize = textures[spr.TextureIndex]->GetSize();

				if (!SpriteFitsInTexture(sprPos, sprSize, texSize))
					return;

				auto texRGBA = reinterpret_cast<const u32*>(rgbaTextures[spr.TextureIndex].get());
				if (texRGBA == nullptr)
					return;

				for (size_t y = 0; y < sprSize.y; y++)
				{
					for (size_t x = 0; x < sprSize.x; x++)
					{
						// NOTE: Also perform texture Y flip
						const u32 texPixel = texRGBA[texSize.x * ((texSize.y - 1) - y - sprPos.y) + (x + sprPos.x)];

						sprRGBA[sprSize.x * y + x] = texPixel;
					}
				}

				const auto fileName = Util::ToLowerCopy(spr.Name) + ".png";
				const auto filePath = IO::Path::Combine(outputDirectory, fileName);

				Util::WriteImage(filePath, sprSize, sprRGBA);
			});
		}

		for (size_t i = 0; i < sprites.size(); i++)
			spriteFutures[i].wait();
	}
}
