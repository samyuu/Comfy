#pragma once
#include "Graphics/Direct3D/D3D_Renderer2D.h"

namespace Editor
{
	class CheckerboardGrid
	{
	public:
		static constexpr float DefaultGridSize = 5.0f;

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

		void Render(Graphics::D3D_Renderer2D* renderer) const;

	private:
	};
}
