#pragma once
#include "Types.h"
#include "Render/D3D11/Texture/Texture.h"
#include <optional>

namespace Comfy::Render
{
	constexpr size_t SpriteTextureSlots = 7;
}

namespace Comfy::Render::Detail
{
	struct SpriteIndices
	{
		u16 TopLeft;
		u16 BottomLeft;
		u16 BottomRight;
		u16 BottomRightCopy;
		u16 TopRight;
		u16 TopLeftCopy;

		static constexpr u32 GetIndexCount() { return sizeof(SpriteIndices) / sizeof(u16); };
	};

	struct SpriteVertex
	{
		vec2 Position;
		vec2 TextureCoordinates;
		vec2 TextureMaskCoordinates;
		u32 Color;
		u32 TextureIndex;
	};

	struct SpriteVertices
	{
		SpriteVertex TopLeft;
		SpriteVertex TopRight;
		SpriteVertex BottomLeft;
		SpriteVertex BottomRight;

	public:
		void SetValues(vec2 position, const vec4& sourceRegion, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4 colors[4], bool setTexCoords);
		static constexpr u32 GetVertexCount() { return sizeof(SpriteVertices) / sizeof(SpriteVertex); };

	public:
		void SetPositionsNoRotation(vec2 position, vec2 size);
		void SetPositions(vec2 position, vec2 size, vec2 origin, float rotation);
		void SetTexCoords(vec2 topLeft, vec2 bottomRight);
		void SetNullTexCoords();
		void SetTexMaskCoords(
			const D3D11::Texture2D* texture, vec2 position, vec2 scale, vec2 origin, float rotation,
			vec2 maskPosition, vec2 maskScale, vec2 maskOrigin, float maskRotation, const vec4& maskSourceRegion);
		void SetColors(const vec4& color);
		void SetColorArray(const vec4 colors[4]);

	public:
		void SetTextureIndices(u32 textureIndex);
	};

	struct SpriteBatch
	{
		u16 Index;
		u16 Count;

		std::array<const D3D11::Texture2D*, SpriteTextureSlots> Textures = {};

		SpriteBatch(u16 index, u16 count) : Index(index), Count(count) {};
	};

	struct SpriteBatchItem
	{
		// NOTE: Never nullptr (WhiteTexture used as a backup)
		const D3D11::Texture2D* Texture;

		// NOTE: May be nullptr
		const D3D11::Texture2D* MaskTexture;

		Graphics::AetBlendMode BlendMode;
		bool DrawTextBorder;

		std::optional<vec2> CheckerboardSize;
	};

	struct SpriteBatchPair
	{
		SpriteBatchItem* Item;
		SpriteVertices* Vertices;
	};
}
