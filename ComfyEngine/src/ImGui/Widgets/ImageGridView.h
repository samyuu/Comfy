#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/CheckerboardTexture.h"

namespace ImGui
{
	class ImageGridView
	{
	public:
		ImageGridView() = default;
		~ImageGridView() = default;

	public:
		void Begin(vec2 childSize = vec2(0.0f, 0.0f));
		void Add(std::string_view name, Comfy::ComfyTextureID image, vec2 imageDimensions, bool flipUV = false);
		void End();

	private:
		CheckerboardTexture checkerboard;

		vec2 availableSize = {};
		ImRect availableScreenRegion = {};

		float perItemSpacing = 0.0f;
		float firstItemOffset = 0.0f;

		vec2 imageBoundingBoxSize = vec2(180.0f, 80.0f);
		vec2 textBoundingBoxSize = vec2(180.0f, 22.0f);
	};
}
