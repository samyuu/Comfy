#pragma once
#include "Types.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	class CheckerboardGrid
	{
	public:
		static constexpr float DefaultGridSize = 5.0f;

	public:
		// NOTE: Pixels between each cell
		float GridSize = DefaultGridSize;

		// NOTE: Top left start position
		vec2 Position = vec2(0.0f, 0.0f);
		// NOTE: Size to be covered
		vec2 Size = vec2(0.0f, 0.0f);

		// NOTE: Primary color
		vec4 Color = vec4(0.15f, 0.15f, 0.15f, 1.0f);
		// NOTE: Secondary color
		vec4 ColorAlt = vec4(0.32f, 0.32f, 0.32f, 1.0f);

		inline void Render(Render::Renderer2D& renderer) const
		{
			renderer.Draw(Render::RenderCommand2D(
				Position,
				Size,
				Color));

			renderer.DrawRectCheckerboard(
				Position,
				vec2(1.0f),
				vec2(0.0f),
				0.0f,
				Size,
				ColorAlt,
				renderer.GetCamera().Zoom / GridSize);
		}

	private:
	};
}
