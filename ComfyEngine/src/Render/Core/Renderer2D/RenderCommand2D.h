#pragma once
#include "Types.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Render
{
	struct RenderCommand2D
	{
	public:
		// TODO: Various constructors
		RenderCommand2D() = default;

		RenderCommand2D(vec2 position, vec2 size, vec4 color) : Position(position), Scale(size)
		{
			SetColor(color);
		}

		RenderCommand2D(const Graphics::Tex* texture, vec2 origin, vec2 position, float rotation, vec2 scale, vec4 sourceRegion, Graphics::AetBlendMode blendMode, float opacity)
			: Texture(texture), Origin(origin), Position(position), Rotation(rotation), Scale(scale), SourceRegion(sourceRegion), BlendMode(blendMode)
		{
			for (auto& cornerColor : CornerColors)
				cornerColor.a = opacity;
		}

		void SetColor(const vec4& newColor)
		{
			for (auto& cornerColor : CornerColors)
				cornerColor = newColor;
		}

	public:
		// NOTE: Can be null to draw a solid color rectangle instead
		const Graphics::Tex* Texture = nullptr;

		vec2 Origin = { 0.0f, 0.0f };
		vec2 Position = { 0.0f, 0.0f };
		float Rotation = 0.0f;
		vec2 Scale = { 1.0f, 1.0f };
		vec4 SourceRegion = { 0.0f, 0.0f, 1.0f, 1.0f };
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Normal;

		// NOTE: Top left, top right, bottom left, bottom right
		std::array<vec4, 4> CornerColors = { vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f) };

		// TODO: Extra flags, texture filter, wrap (problematic with atlases), etc.
		bool DrawTextBorder = false;
	};

}
