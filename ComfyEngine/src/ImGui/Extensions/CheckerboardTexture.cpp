#include "CheckerboardTexture.h"

namespace ImGui
{
	namespace
	{
		u32& GetRGBAPixel(int width, void* rgbaPixels, int x, int y)
		{
			return reinterpret_cast<u32*>(rgbaPixels)[(width * y) + x];
		}
	}

	CheckerboardTexture::CheckerboardTexture(vec4 color, vec4 colorAlt, int gridSize)
	{
		const auto colors = std::array { ColorConvertFloat4ToU32(color), ColorConvertFloat4ToU32(colorAlt) };

		auto& baseMipMap = texture.MipMapsArray.emplace_back().emplace_back();
		baseMipMap.Size = ivec2(gridSize * 2);
		baseMipMap.Format = Comfy::Graphics::TextureFormat::RGBA8;
		baseMipMap.DataSize = baseMipMap.Size.x * baseMipMap.Size.y * sizeof(u32);
		baseMipMap.Data = std::make_unique<u8[]>(baseMipMap.DataSize);

		bool altColorX = false, altColorY;
		for (int x = 0; x < baseMipMap.Size.x; x++)
		{
			if (x % gridSize == 0)
				altColorY = !(altColorX ^= true);

			for (int y = 0; y < baseMipMap.Size.y; y++)
			{
				altColorY ^= (y % gridSize == 0);
				GetRGBAPixel(baseMipMap.Size.x, baseMipMap.Data.get(), x, y) = colors[altColorY];
			}
		}
	}

	void CheckerboardTexture::AddToDrawList(ImDrawList* drawList, ImRect region)
	{
		const auto uvScale = region.GetSize() / vec2(texture.GetSize());
		const auto uv = std::array { UV0 * uvScale, UV1 * uvScale };

		drawList->AddImage(texture, region.GetTL(), region.GetBR(), uv[0], uv[1]);
	}
}
