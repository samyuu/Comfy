#pragma once
#include "Types.h"
#include "Render/Render.h"
#include <future>

namespace Comfy::Studio::Editor
{
	using AsyncImageFileFlags = u32;
	enum AsyncImageFileFlagsEnum : AsyncImageFileFlags
	{
		AsyncImageFileFlags_None = 0,
		AsyncImageFileFlags_FlipY = 1 << 0,
		AsyncImageFileFlags_TransparentBorder = 1 << 1,
		AsyncImageFileFlags_TransparentBorderNoSprAdjust = 1 << 2,
	};

	class AsyncLoadedImageFile
	{
	public:
		AsyncLoadedImageFile(AsyncImageFileFlags flags = AsyncImageFileFlags_FlipY);
		~AsyncLoadedImageFile();

	public:
		void TryLoad(std::string_view relativeOrAbsolutePath, std::string_view basePathOrDirectory = "");

		Render::TexSprView GetTexSprView();

		bool IsAsyncLoaded() const;
		bool IsAsyncLoading() const;

	private:
		void GetMoveFuture();
		void LoadAsync();

	private:
		AsyncImageFileFlags flags = {};

		std::unique_ptr<Graphics::Tex> texture = nullptr;
		Graphics::Spr sprite = {};

		std::string lastSetRelativeOrAbsoultePath, lastSetBasePathOrDirectory, absolutePath;
		std::future<std::pair<std::unique_ptr<Graphics::Tex>, Graphics::Spr>> future;
	};
}
