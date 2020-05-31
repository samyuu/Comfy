#include "FontRenderer.h"
#include "Renderer2D.h"
#include "ImGui/Extensions/ImGuiExtensions.h"

namespace Comfy::Render
{
	template <typename Func>
	void ForEachUTF8Char32(std::string_view inputText, Func func)
	{
		if (inputText.empty())
			return;

		const char* textStart = ImGui::StringViewStart(inputText);
		const char* textEnd = ImGui::StringViewEnd(inputText);

		while (textStart < textEnd)
		{
			unsigned int outChar;
			const auto byteLength = ImTextCharFromUtf8(&outChar, textStart, textEnd);

			if (byteLength < 1 || !func(static_cast<char32_t>(outChar)))
				return;

			textStart += byteLength;
		}
	}

	FontRenderer::FontRenderer(Renderer2D& renderer) : renderer2D(renderer)
	{
	}

	void FontRenderer::Draw(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, vec3 color)
	{
		Render::RenderCommand2D command;
		command.SetColor(vec4(color, transform.Opacity));
		DrawInternal(font, text, transform, command);
	}

	void FontRenderer::DrawBorder(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, vec3 color)
	{
		Render::RenderCommand2D command;
		command.DrawTextBorder = true;
		command.SetColor(vec4(color, transform.Opacity));
		DrawInternal(font, text, transform, command);
	}

	void FontRenderer::DrawShadow(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, vec3 color, vec4 shadowColor, vec2 offset)
	{
		Render::RenderCommand2D command;

		auto shadowTransform = transform;
		shadowTransform.Position += offset;

		shadowColor.a *= transform.Opacity;
		command.SetColor(shadowColor);
		DrawInternal(font, text, shadowTransform, command);

		command.SetColor(vec4(color, transform.Opacity));
		DrawInternal(font, text, transform, command);
	}

	vec2 FontRenderer::Measure(const Graphics::BitmapFont& font, std::string_view text) const
	{
		const auto fontSize = vec2(font.GetFontSize());

		const std::array<vec2, 2> glpyhAdvance =
		{
			fontSize,
			fontSize * vec2(0.5f, 1.0f),
		};

		vec2 cursorOffset = {}, textSize = {};
		ForEachUTF8Char32(text, [&](char32_t character)
		{
			if (ProcessControlCharacters(character, cursorOffset, fontSize))
				return true;

			const auto glyph = font.GetGlyph(character);
			if (glyph == nullptr)
				return true;

			cursorOffset.x += glpyhAdvance[glyph->IsNarrow].x;
			if (cursorOffset.x > textSize.x) textSize.x = cursorOffset.x;
			if (cursorOffset.y > textSize.y) textSize.y = cursorOffset.y;
			return true;
		});

		return vec2(textSize.x, textSize.y + fontSize.y);
	}

	void FontRenderer::DrawInternal(const Graphics::BitmapFont& font, std::string_view text, const Graphics::Transform2D& transform, RenderCommand2D& command)
	{
		const auto fontSize = vec2(font.GetFontSize());
		const auto glyphSize = vec2(font.GetGlyphSize());

		command.Texture = font.Texture.get();
		command.Rotation = transform.Rotation;
		command.Scale = transform.Scale;
		command.SourceRegion.z = fontSize.x;
		command.SourceRegion.w = fontSize.y;

		const std::array<vec2, 2> glpyhAdvance =
		{
			fontSize,
			fontSize * vec2(0.5f, 1.0f),
		};

		vec2 cursorOffset = {};
		ForEachUTF8Char32(text, [&](char32_t character)
		{
			if (ProcessControlCharacters(character, cursorOffset, fontSize))
				return true;

			const auto glyph = font.GetGlyph(character);
			if (glyph == nullptr)
				return true;

			command.Position = transform.Position;
			command.Origin = transform.Origin - cursorOffset;
			command.SourceRegion.x = (glyphSize.x * glyph->Row);
			command.SourceRegion.y = (glyphSize.y * glyph->Column);
			renderer2D.Draw(command);

			cursorOffset.x += glpyhAdvance[glyph->IsNarrow].x;
			return true;
		});
	}

	bool FontRenderer::ProcessControlCharacters(char32_t character, vec2& cursorOffset, vec2 fontSize) const
	{
		if (character == U'\r')
			return true;

		if (character == U'\n')
		{
			cursorOffset.x = 0.0f;
			cursorOffset.y += fontSize.y;
			return true;
		}

		if (character == U'\t')
		{
			constexpr float tabWidth = 2.0f;
			cursorOffset.x += (fontSize.x * tabWidth);
			return true;
		}

		if (character == U' ')
		{
			constexpr float spaceWidth = 0.5f;
			cursorOffset.x += (fontSize.x * spaceWidth);
			return true;
		}

		return false;
	}
}
