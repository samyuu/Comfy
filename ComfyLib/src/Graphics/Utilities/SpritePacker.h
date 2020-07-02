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
		None, BC1, BC2, BC3, BC4, BC5, Unknown,
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
			std::optional<u32> BackgroundColor = 0x00000000; // 0xFFFF00FF;

			ivec2 MaxTextureSize = ivec2(2048, 1024);

			// NOTE: Number of pixels at each side
			ivec2 SpritePadding = ivec2(2, 2);

			bool AllowYCbCrTextures = true;
			bool GenerateMipMaps = false;

			bool PowerOfTwoTextures = true;
			bool FlipTexturesY = true;
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

		std::shared_ptr<Tex> CreateTexFromMarkup(const SprTexMarkup& texMarkup);
		std::unique_ptr<u8[]> CreateMergedTexMarkupRGBAPixels(const SprTexMarkup& texMarkup);

		const char* GetCompressionName(TextureFormat format) const;
		std::string FormatTextureName(MergeType merge, TextureFormat format, size_t index) const;
	};
}
