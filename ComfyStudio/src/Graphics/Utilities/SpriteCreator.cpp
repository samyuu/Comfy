#include "SpriteCreator.h"
#include <numeric>

namespace Comfy::Graphics::Utilities
{
	namespace
	{
		constexpr size_t RGBABytesPerPixel = 4;
		constexpr uint32_t TransparentRGBA = 0x00000000;

		constexpr int Area(const ivec2& size)
		{
			return (size.x * size.y);
		}

		constexpr bool Intersects(const ivec4& boxA, const ivec4& boxB)
		{
			return
				(boxB.x < boxA.x + boxA.z) && (boxA.x < (boxB.x + boxB.z)) &&
				(boxB.y < boxA.y + boxA.w) && (boxA.y < boxB.y + boxB.w);
		}

		constexpr bool Contains(const ivec4& boxA, const ivec4& boxB)
		{
			return
				(boxA.x <= boxB.x) && ((boxB.x + boxB.z) <= (boxA.x + boxA.z)) &&
				(boxA.y <= boxB.y) && ((boxB.y + boxB.w) <= (boxA.y + boxA.w));
		}

		bool FitsInsideTexture(const ivec4& textureBox, const std::vector<SprMarkupBox>& existingSprites, ivec4 spriteBox)
		{
			if (!Contains(textureBox, spriteBox))
				return false;

			for (auto& existingSprite : existingSprites)
			{
				if (Intersects(existingSprite.Box, spriteBox))
					return false;
			}

			return true;
		}

		constexpr vec4 GetTexelRegionFromPixelRegion(const vec4& pixelRegion, vec2 textureSize)
		{
			vec4 texelRegion =
			{
				(pixelRegion.x / textureSize.x),
				(pixelRegion.y / textureSize.y),
				(pixelRegion.z / textureSize.x),
				(pixelRegion.w / textureSize.y),
			};
			return texelRegion;
		}

		uint32_t& GetPixel(int width, void* rgbaPixels, int x, int y)
		{
			return reinterpret_cast<uint32_t*>(rgbaPixels)[(width * y) + x];
		}

		const uint32_t& GetPixel(int width, const void* rgbaPixels, int x, int y)
		{
			return reinterpret_cast<const uint32_t*>(rgbaPixels)[(width * y) + x];
		}

		void FlipTextureY(ivec2 size, void* rgbaPixels)
		{
			for (int y = 0; y < size.y / 2; y++)
			{
				for (int x = 0; x < size.x; x++)
				{
					uint32_t& pixel = GetPixel(size.x, rgbaPixels, x, y);
					uint32_t& flippedPixel = GetPixel(size.x, rgbaPixels, x, size.y - 1 - y);

					std::swap(pixel, flippedPixel);
				}
			}
		}
	}

	UniquePtr<SprSet> SpriteCreator::Create(const std::vector<SprMarkup>& sprMarkups)
	{
		auto result = MakeUnique<SprSet>();
		result->TexSet = MakeUnique<TexSet>();

		SprSet& sprSet = *result;
		TexSet& texSet = *sprSet.TexSet;

		sprSet.Flags = 0;
		sprSet.Sprites.reserve(sprMarkups.size());

		auto texMarkups = MergeTextures(sprMarkups);
		texSet.Textures.reserve(texMarkups.size());

		for (int texIndex = 0; texIndex < static_cast<int>(texMarkups.size()); texIndex++)
		{
			const auto& texMarkup = texMarkups[texIndex];
			for (const auto& spriteBox : texMarkup.SpriteBoxes)
			{
				const auto& sprMarkup = *spriteBox.Markup;
				auto& spr = sprSet.Sprites.emplace_back();
				spr.TextureIndex = texIndex;
				spr.Rotate = 0;
				spr.PixelRegion = spriteBox.Box;
				spr.TexelRegion = GetTexelRegionFromPixelRegion(spr.PixelRegion, texMarkup.Size);
				spr.Name = sprMarkup.Name;
				spr.Extra.Flags = 0;
				spr.Extra.ScreenMode = sprMarkup.ScreenMode;
			}

			texSet.Textures.push_back(CreateTexFromMarkup(texMarkup));
		}

		// const auto totalWastedPixels = std::accumulate(texMarkups.begin(), texMarkups.end(), 0, [&](auto& pixels, const auto& texMarkup) { return pixels + texMarkup.RemainingFreePixels; });
		// printf(__FUNCTION__"(): totalWastedPixels = %d\n", totalWastedPixels);

		return result;
	}

	std::vector<SprTexMarkup> SpriteCreator::MergeTextures(const std::vector<SprMarkup>& sprMarkups)
	{
		const auto sizeSortedSprMarkups = SortByArea(sprMarkups);
		std::vector<SprTexMarkup> texMarkups;

		size_t noMergeIndex = 0, mergeIndex = 0;

		for (const auto* sprMarkupPtr : sizeSortedSprMarkups)
		{
			auto addTexMarkup = [&](const ivec2 texSize, const auto& sprMarkup, TextureFormat format, MergeType merge, size_t& inOutIndex)
			{
				auto& texMarkup = texMarkups.emplace_back();
				texMarkup.Size = texSize;
				texMarkup.Format = format;
				texMarkup.Merge = merge;
				texMarkup.SpriteBoxes.push_back({ &sprMarkup, ivec4(ivec2(0, 0), sprMarkup.Size) });
				texMarkup.Name = FormatTextureName(texMarkup.Merge, texMarkup.Format, inOutIndex++);
				texMarkup.RemainingFreePixels = Area(texMarkup.Size) - Area(sprMarkup.Size);
			};

			const auto& sprMarkup = *sprMarkupPtr;
			const bool largerThanMax = (sprMarkup.Size.x > settings.MaxTextureSize.x || sprMarkup.Size.y > settings.MaxTextureSize.y);

			if ((sprMarkup.Flags & SprMarkupFlags_NoMerge) || largerThanMax)
			{
				addTexMarkup(sprMarkup.Size, sprMarkup, TextureFormat::RGBA8, MergeType::NoMerge, noMergeIndex);
			}
			else
			{
				auto tryAddSpriteToExistingTexture = [&]() -> bool
				{
					for (auto& existingTexMarkup : texMarkups)
					{
						if (existingTexMarkup.Merge == MergeType::NoMerge || existingTexMarkup.RemainingFreePixels < Area(sprMarkup.Size))
							continue;

						const ivec4 texBox = ivec4(ivec2(0, 0), existingTexMarkup.Size);
						ivec4 sprBox = ivec4(ivec2(0, 0), sprMarkup.Size);

						constexpr int stepSize = 4;
						for (sprBox.y = 0; sprBox.y < existingTexMarkup.Size.y; sprBox.y += stepSize)
						{
							for (sprBox.x = 0; sprBox.x < existingTexMarkup.Size.x; sprBox.x += stepSize)
							{
								if (FitsInsideTexture(texBox, existingTexMarkup.SpriteBoxes, sprBox))
								{
									existingTexMarkup.SpriteBoxes.push_back({ &sprMarkup, sprBox });
									existingTexMarkup.RemainingFreePixels -= Area(sprMarkup.Size);
									return true;
								}
							}
						}
					}

					return false;
				};

				if (!tryAddSpriteToExistingTexture())
					addTexMarkup(settings.MaxTextureSize, sprMarkup, TextureFormat::RGBA8, MergeType::Merge, mergeIndex);
			}
		}

		return texMarkups;
	}

	std::vector<const SprMarkup*> SpriteCreator::SortByArea(const std::vector<SprMarkup>& sprMarkups) const
	{
		std::vector<const SprMarkup*> result;
		result.reserve(sprMarkups.size());

		for (auto& sprMarkup : sprMarkups)
			result.push_back(&sprMarkup);

		std::sort(result.begin(), result.end(), [](auto& sprA, auto& sprB)
		{
			return Area(sprA->Size) > Area(sprB->Size);
		});

		return result;
	}

	RefPtr<Tex> SpriteCreator::CreateTexFromMarkup(const SprTexMarkup& texMarkup)
	{
		auto margedRGBAPixels = CreateMergedTexMarkupRGBAPixels(texMarkup);

		auto tex = MakeRef<Tex>();
		tex->Name = texMarkup.Name;
		auto& mipMaps = tex->MipMapsArray.emplace_back();
		auto& baseMipMap = mipMaps.emplace_back();
		baseMipMap.Format = TextureFormat::RGBA8;
		baseMipMap.Size = texMarkup.Size;
		baseMipMap.DataSize = Area(texMarkup.Size) * RGBABytesPerPixel;
		// TODO: In the future only move once all mipmaps have been generated
		baseMipMap.Data = std::move(margedRGBAPixels);

		return tex;
	}

	UniquePtr<uint8_t[]> SpriteCreator::CreateMergedTexMarkupRGBAPixels(const SprTexMarkup& texMarkup)
	{
		const size_t texDataSize = Area(texMarkup.Size) * RGBABytesPerPixel;
		auto texData = MakeUnique<uint8_t[]>(texDataSize);

		if (texMarkup.Merge == MergeType::NoMerge)
		{
			std::memcpy(texData.get(), texMarkup.SpriteBoxes.front().Markup->RGBAPixels, texDataSize);
		}
		else
		{
			constexpr uint32_t dummyColor = 0xFFFF00FF;
			for (size_t i = 0; i < texDataSize / RGBABytesPerPixel; i++)
				reinterpret_cast<uint32_t*>(texData.get())[i] = dummyColor;

			for (auto& sprBox : texMarkup.SpriteBoxes)
			{
				for (int x = 0; x < sprBox.Markup->Size.x; x++)
				{
					for (int y = 0; y < sprBox.Markup->Size.y; y++)
					{
						const uint32_t& sprPixel = GetPixel(sprBox.Box.z, sprBox.Markup->RGBAPixels, x, y);
						uint32_t& texPixel = GetPixel(texMarkup.Size.x, texData.get(), x + sprBox.Box.x, y + sprBox.Box.y);

						texPixel = sprPixel;
					}
				}
			}
		}

		if (settings.FlipY)
			FlipTextureY(texMarkup.Size, texData.get());

		return texData;
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

	std::string SpriteCreator::FormatTextureName(MergeType merge, TextureFormat format, size_t index) const
	{
		const char* mergeString = (merge == MergeType::Merge) ? "MERGE" : "NOMERGE";
		const char* compressionString = GetCompressionName(format);

		char nameBuffer[32];
		sprintf_s(nameBuffer, "%s_%s_%03zu", mergeString, compressionString, index);
		return nameBuffer;
	}
}
