#pragma once
#include "Types.h"
#include "Graphics/D3D11/Texture/Texture.h"

namespace Comfy::Graphics::D3D11
{
	struct SpriteIndices
	{
		u16 TopLeft;
		u16 BottomLeft;
		u16 BottomRight;
		u16 BottomRightCopy;
		u16 TopRight;
		u16 TopLeftCopy;

	public:
		static inline constexpr u32 GetIndexCount() { return sizeof(SpriteIndices) / sizeof(u16); };
	};
	
	struct SpriteVertex
	{
		// TODO: Consider using 16-bit texture coordinates
		vec2 Position;
		vec2 TextureCoordinates;
		vec2 TextureMaskCoordinates;
		u32 Color;
	};

	struct SpriteVertices
	{
		SpriteVertex TopLeft;
		SpriteVertex TopRight;
		SpriteVertex BottomLeft;
		SpriteVertex BottomRight;

	public:
		void SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color);
		void SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4 colors[4]);
		static inline constexpr u32 GetVertexCount() { return sizeof(SpriteVertices) / sizeof(SpriteVertex); };

	public:
		void SetPositions(const vec2& position, const vec2& size);
		void SetPositions(const vec2& position, const vec2& size, const vec2& origin, float rotation);
		void SetTexCoords(const vec2& topLeft, const vec2& bottomRight);
		void SetTexMaskCoords(
			const Texture2D* texture, const vec2& position, const vec2& scale, const vec2& origin, float rotation,
			const vec2& maskPosition, const vec2& maskScale, const vec2& maskOrigin, float maskRotation, const vec4& maskSourceRegion);
		void SetColors(const vec4& color);
		void SetColorArray(const vec4 colors[4]);
	};

	struct SpriteBatch
	{
		u16 Index;
		u16 Count;

		SpriteBatch(u16 index, u16 count) : Index(index), Count(count) {};
	};

	struct SpriteBatchItem
	{
		const Texture2D* Texture;
		const Texture2D* MaskTexture;
		AetBlendMode BlendMode;
		vec2 CheckerboardSize;

		void SetValues(const Texture2D* texture, const Texture2D* alphaMask = nullptr, AetBlendMode blendMode = AetBlendMode::Normal);
	};

	struct SpriteBatchPair
	{
		SpriteBatchItem* Item;
		SpriteVertices* Vertices;
	};
}
