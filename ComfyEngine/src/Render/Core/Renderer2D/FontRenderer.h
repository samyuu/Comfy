#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Graphics/Auth2D/Font/FontMap.h"
#include "Graphics/Auth2D/Transform2D.h"

namespace Comfy::Render
{
	class Renderer2D;
	struct RenderCommand2D;

	class FontRenderer : NonCopyable
	{
	public:
		static constexpr vec3 DefaultColor = vec3(1.0f, 1.0f, 1.0f);
		static constexpr vec4 DefaultShadowColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		static constexpr vec2 DefaultShadowOffset = vec2(2.0f, 2.0f);

	public:
		FontRenderer(Renderer2D& renderer);
		~FontRenderer() = default;

		// NOTE: These are only valid between a Rendere2D Being() and End() block
	public:
		void Draw(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, vec3 color = DefaultColor);
		void DrawBorder(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, vec3 color = DefaultColor);
		void DrawShadow(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, vec3 color = DefaultColor, vec4 shadowColor = DefaultShadowColor, vec2 offset = DefaultShadowOffset);

	public:
		vec2 Measure(const Graphics::BitmapFont& font, std::string_view text) const;

	private:
		void DrawInternal(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, RenderCommand2D& command);
		bool ProcessControlCharacters(char32_t character, vec2& cursorOffset, vec2 fontSize) const;

	private:
		Renderer2D& renderer2D;
	};
}
