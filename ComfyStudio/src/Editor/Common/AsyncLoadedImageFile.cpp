#include "AsyncLoadedImageFile.h"
#include "Misc/ImageHelper.h"
#include "IO/Path.h"
#include "Graphics/Utilities/TextureCompression.h"

namespace Comfy::Studio::Editor
{
	AsyncLoadedImageFile::AsyncLoadedImageFile(AsyncImageFileFlags flags) : flags(flags)
	{
	}

	AsyncLoadedImageFile::~AsyncLoadedImageFile()
	{
		if (future.valid())
			GetMoveFuture();
		texture = nullptr;
	}

	void AsyncLoadedImageFile::TryLoad(std::string_view relativeOrAbsolutePath, std::string_view basePathOrDirectory)
	{
		if (relativeOrAbsolutePath == lastSetRelativeOrAbsoultePath && basePathOrDirectory == lastSetBasePathOrDirectory)
			return;

		auto newAbsolutePath = IO::Path::ResolveRelativeTo(relativeOrAbsolutePath, basePathOrDirectory);
		if (newAbsolutePath == absolutePath)
			return;

		absolutePath = newAbsolutePath;
		lastSetRelativeOrAbsoultePath = relativeOrAbsolutePath;
		lastSetBasePathOrDirectory = basePathOrDirectory;

		if (future.valid())
			GetMoveFuture();
		LoadAsync();
	}

	Render::TexSprView AsyncLoadedImageFile::GetTexSprView()
	{
		if (future.valid() && future._Is_ready())
			GetMoveFuture();
		return (texture != nullptr) ? Render::TexSprView { texture.get(), &sprite } : Render::TexSprView { nullptr, nullptr };
	}

	bool AsyncLoadedImageFile::IsAsyncLoaded() const
	{
		return (!future.valid() || future._Is_ready());
	}

	bool AsyncLoadedImageFile::IsAsyncLoading() const
	{
		return (future.valid() && !future._Is_ready());
	}

	void AsyncLoadedImageFile::GetMoveFuture()
	{
		auto[tex, spr] = future.get();
		texture = std::move(tex);
		sprite = std::move(spr);
	}

	void AsyncLoadedImageFile::LoadAsync()
	{
		if (absolutePath.empty())
		{
			texture = {};
			sprite = {};
			future = {};
			return;
		}

		future = std::async(std::launch::async, [this]() -> std::pair<std::unique_ptr<Graphics::Tex>, Graphics::Spr>
		{
			ivec2 imageSize = {};
			std::unique_ptr<u8[]> rgbaPixels = {};
			if (!Util::ReadImage(absolutePath, imageSize, rgbaPixels))
				return {};

			constexpr auto perSidePadding = 1;
			if (flags & AsyncImageFileFlags_TransparentBorder)
			{
				auto paddedSize = imageSize + ivec2(perSidePadding * 2);
				auto paddedPixels = std::make_unique<u8[]>(paddedSize.x * paddedSize.y * 4);

				auto srcPixels = reinterpret_cast<u32*>(rgbaPixels.get());
				auto dstPixels = reinterpret_cast<u32*>(paddedPixels.get());

				auto cutAlpha = [](u32 pixel) -> u32 { return (pixel & 0x00FFFFFF); };
				auto getPixel = [](i32 width, u32* rgbaPixels, i32 x, i32 y) -> u32& { return rgbaPixels[(width * y) + x]; };

				const auto cornerBottomRight = paddedSize - perSidePadding;
				for (i32 x = 0; x < perSidePadding; x++)
				{
					for (i32 y = 0; y < perSidePadding; y++)
					{
						const auto topLeft = ivec2(x, y);
						const auto bottomRight = cornerBottomRight + ivec2(x, y);

						// NOTE: Top left / bottom left / top right / bottom right
						getPixel(paddedSize.x, dstPixels, topLeft.x, topLeft.y) = cutAlpha(getPixel(imageSize.x, srcPixels, 0, 0));
						getPixel(paddedSize.x, dstPixels, topLeft.x, bottomRight.y) = cutAlpha(getPixel(imageSize.x, srcPixels, 0, imageSize.y - 1));
						getPixel(paddedSize.x, dstPixels, bottomRight.x, topLeft.y) = cutAlpha(getPixel(imageSize.x, srcPixels, imageSize.x - 1, 0));
						getPixel(paddedSize.x, dstPixels, bottomRight.x, bottomRight.y) = cutAlpha(getPixel(imageSize.x, srcPixels, imageSize.x - 1, imageSize.y - 1));
					}
				}

				// NOTE: Top / bottom / left / right
				for (i32 x = perSidePadding; x < paddedSize.x - perSidePadding; x++)
				{
					for (i32 y = 0; y < perSidePadding; y++)
						getPixel(paddedSize.x, dstPixels, x, y) = cutAlpha(getPixel(imageSize.x, srcPixels, x - perSidePadding, 0));
					for (i32 y = paddedSize.y - perSidePadding; y < paddedSize.y; y++)
						getPixel(paddedSize.x, dstPixels, x, y) = cutAlpha(getPixel(imageSize.x, srcPixels, x - perSidePadding, imageSize.y - 1));
				}
				for (i32 y = perSidePadding; y < paddedSize.y - perSidePadding; y++)
				{
					for (i32 x = 0; x < perSidePadding; x++)
						getPixel(paddedSize.x, dstPixels, x, y) = cutAlpha(getPixel(imageSize.x, srcPixels, 0, y - perSidePadding));
					for (i32 x = paddedSize.x - perSidePadding; x < paddedSize.x; x++)
						getPixel(paddedSize.x, dstPixels, x, y) = cutAlpha(getPixel(imageSize.x, srcPixels, imageSize.x - 1, y - perSidePadding));
				}

				for (i32 y = 0; y < imageSize.y; y++)
					for (i32 x = 0; x < imageSize.x; x++)
						dstPixels[(y + perSidePadding) * paddedSize.x + (x + perSidePadding)] = srcPixels[y * imageSize.x + x];

				imageSize = paddedSize;
				rgbaPixels = std::move(paddedPixels);
			}

			const auto dataByteSize = Graphics::Utilities::TextureFormatByteSize(imageSize, Graphics::TextureFormat::RGBA8);

			if (flags & AsyncImageFileFlags_FlipY)
				Graphics::Utilities::FlipTextureBufferY(imageSize, rgbaPixels.get(), Graphics::TextureFormat::RGBA8, dataByteSize);

			auto tex = std::make_unique<Graphics::Tex>();
			auto& baseMip = tex->MipMapsArray.emplace_back().emplace_back();
			baseMip.Size = imageSize;
			baseMip.Format = Graphics::TextureFormat::RGBA8;
			baseMip.DataSize = static_cast<u32>(dataByteSize);
			baseMip.Data = std::move(rgbaPixels);

			Graphics::Spr spr = {};
			spr.TexelRegion = vec4(0.0f, 0.0f, 1.0f, 1.0f);
			spr.PixelRegion = vec4(0.0f, 0.0f, imageSize.x, imageSize.y);
			spr.Name = IO::Path::GetFileName(absolutePath, false);
			spr.Extra.ScreenMode = Graphics::ScreenMode::HDTV1080;

			if ((flags & AsyncImageFileFlags_TransparentBorder) && !(flags & AsyncImageFileFlags_TransparentBorderNoSprAdjust))
			{
				spr.PixelRegion.x += perSidePadding;
				spr.PixelRegion.y += perSidePadding;
				spr.PixelRegion.z -= (perSidePadding * 2.0f);
				spr.PixelRegion.w -= (perSidePadding * 2.0f);
			}

			return std::make_pair(std::move(tex), std::move(spr));
		});
	}
}
