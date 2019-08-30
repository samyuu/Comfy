#pragma once
#include "Graphics/Auth2D/Renderer2D.h"

namespace Editor
{
	class CheckerboardGrid
	{
	public:
		float GridSize = 1.0f;
		vec2 Size = vec2(0.0f, 0.0f);
		vec2 Position = vec2(0.0f, 0.0f);

		vec4 Color = vec4(0.15f, 0.15f, 0.15f, 1.0f);
		vec4 ColorAlt = vec4(0.32f, 0.32f, 0.32f, 1.0f);

		void Render(Graphics::Auth2D::Renderer2D* renderer) const;

	private:
	};
}