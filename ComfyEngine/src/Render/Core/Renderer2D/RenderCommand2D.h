#pragma once
#include "Types.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Render
{
	struct TexSamplerView
	{
		TexSamplerView() = default;

		TexSamplerView(const Graphics::Tex* texture)
			: Texture(texture)
		{
		}

		TexSamplerView(const Graphics::Tex* texture, Graphics::TextureAddressMode addressU, Graphics::TextureAddressMode addressV, Graphics::TextureFilter filter)
			: Texture(texture), AddressU(addressU), AddressV(addressV), Filter(filter)
		{
		}

		const Graphics::Tex* Texture = nullptr;
		Graphics::TextureAddressMode AddressU = Graphics::TextureAddressMode::ClampBorder;
		Graphics::TextureAddressMode AddressV = Graphics::TextureAddressMode::ClampBorder;
		Graphics::TextureFilter Filter = Graphics::TextureFilter::Linear;

		operator bool() const { return (Texture != nullptr); }
		bool operator==(const TexSamplerView& other) const { return (Texture == other.Texture) && (AddressU == other.AddressU) && (AddressV == other.AddressV); }
		bool operator!=(const TexSamplerView& other) const { return !(*this == other); }
	};

	struct RenderCommand2D
	{
	public:
		// TODO: Various constructors
		RenderCommand2D() = default;

		RenderCommand2D(vec2 position, vec2 size, vec4 color) : Position(position), Scale(size)
		{
			SetColor(color);
		}

		RenderCommand2D(TexSamplerView texView, vec2 origin, vec2 position, float rotation, vec2 scale, vec4 sourceRegion, Graphics::AetBlendMode blendMode, float opacity)
			: TexView(texView), Origin(origin), Position(position), Rotation(rotation), Scale(scale), SourceRegion(sourceRegion), BlendMode(blendMode)
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
		TexSamplerView TexView = nullptr;

		vec2 Origin = { 0.0f, 0.0f };
		vec2 Position = { 0.0f, 0.0f };
		float Rotation = 0.0f;
		vec2 Scale = { 1.0f, 1.0f };
		vec4 SourceRegion = { 0.0f, 0.0f, 1.0f, 1.0f };
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Normal;

		// NOTE: Top left, top right, bottom left, bottom right
		std::array<vec4, 4> CornerColors = { vec4(1.0f), vec4(1.0f), vec4(1.0f), vec4(1.0f) };

		bool DrawTextBorder = false;
	};

}
