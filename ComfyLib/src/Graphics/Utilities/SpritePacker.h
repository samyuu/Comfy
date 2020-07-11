#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include <functional>
#include <optional>

namespace Comfy::Graphics::Utilities
{
	enum class MergeType
	{
		NoMerge,
		Merge,
		Count
	};

	enum class CompressionType
	{
		NoComp,
		D5Comp,
		BC4Comp,
		BC5Comp,
		UnkComp,
		Count
	};

	using SprMarkupFlags = u32;
	enum SprMarkupFlagsEnum : SprMarkupFlags
	{
		SprMarkupFlags_None = 0,
		SprMarkupFlags_NoMerge = (1 << 0),
		SprMarkupFlags_Compress = (1 << 1),
	};

	struct SprMarkup
	{
		std::string Name;
		ivec2 Size;
		const void* RGBAPixels;
		ScreenMode ScreenMode;
		SprMarkupFlags Flags;
	};

	struct SprMarkupBox
	{
		const SprMarkup* Markup;
		ivec4 Box;
	};

	struct SprTexMarkup
	{
		std::string Name;
		ivec2 Size;
		TextureFormat OutputFormat;
		MergeType Merge;
		std::vector<SprMarkupBox> SpriteBoxes;
		int RemainingFreePixels;
	};

	class SpritePacker : NonCopyable
	{
	public:
		struct ProgressData
		{
			u32 Sprites, SpritesTotal;
		};

		using ProgressCallback = std::function<void(SpritePacker&, ProgressData)>;

	public:
		SpritePacker() = default;
		SpritePacker(ProgressCallback callback);
		~SpritePacker() = default;

	public:
		struct SettingsData
		{
			// NOTE: Set to 0xFFFF00FF for debugging but fully transparent by default to avoid cross sprite boundary block compression artifacts
			std::optional<u32> BackgroundColor = 0x00000000;

			// NOTE: If any of the input sprites is larger than this threshold the size of the texture will be set to match that of the sprite
			ivec2 MaxTextureSize = ivec2(2048, 1024);

			// NOTE: Number of pixels at each side
			ivec2 SpritePadding = ivec2(2, 2);

			// NOTE: Generally higher quallity than block compression on its own at the cost of additional encoding and decoding time
			bool AllowYCbCrTextures = true;

			// NOTE: Required for texture block compression, textures will stay uncompressed if not set
			bool PowerOfTwoTextures = true;

			// NOTE: Flip to follow the OpenGL texture convention
			bool FlipTexturesY = true;

			// NOTE: Unless the host application wants to run multiple object instances on separate threads itself...?
			bool Multithreaded = true;
		} Settings;

		std::unique_ptr<SprSet> Create(const std::vector<SprMarkup>& sprMarkups);

	protected:
		ProgressData currentProgress = {};
		ProgressCallback progressCallback;

		void ReportCurrentProgress();

	protected:
		TextureFormat DetermineSprOutputFormat(const SprMarkup& sprMarkup) const;

		std::vector<SprTexMarkup> MergeTextures(const std::vector<SprMarkup>& sprMarkups);
		std::vector<const SprMarkup*> SortByArea(const std::vector<SprMarkup>& sprMarkups) const;

		std::pair<SprTexMarkup*, ivec4> FindFittingTexMarkupToPlaceSprIn(const SprMarkup& sprToPlace, TextureFormat sprOutputFormat, std::vector<SprTexMarkup>& existingTexMarkups);
		void AdjustTexMarkupSizes(std::vector<SprTexMarkup>& texMarkups);

		std::shared_ptr<Tex> CreateCompressTexFromMarkup(const SprTexMarkup& texMarkup);
		std::unique_ptr<u8[]> CreateMergedTexMarkupRGBAPixels(const SprTexMarkup& texMarkup);

		CompressionType GetCompressionType(TextureFormat format) const;
		const char* GetMergeName(MergeType merge) const;
		const char* GetCompressionName(CompressionType compression) const;
		std::string FormatTextureName(MergeType merge, CompressionType compression, size_t index) const;
	};
}
