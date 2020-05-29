#pragma once
#include "Types.h"
#include "Render/D3D11/Texture/Texture.h"

#define COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH

#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
namespace Comfy::Render
{
	constexpr size_t SpriteTextureSlots = 7;
}
#endif

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
#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
		u32 TextureIndex;
#endif
	};

	struct SpriteVertices
	{
		SpriteVertex TopLeft;
		SpriteVertex TopRight;
		SpriteVertex BottomLeft;
		SpriteVertex BottomRight;

	public:
		void SetValues(vec2 position, const vec4& sourceRegion, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4& color);
		void SetValues(vec2 position, const vec4& sourceRegion, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4 colors[4]);
		static constexpr u32 GetVertexCount() { return sizeof(SpriteVertices) / sizeof(SpriteVertex); };

	public:
		void SetPositions(vec2 position, vec2 size);
		void SetPositions(vec2 position, vec2 size, vec2 origin, float rotation);
		void SetTexCoords(vec2 topLeft, vec2 bottomRight);
		void SetTexMaskCoords(
			const D3D11::Texture2D* texture, vec2 position, vec2 scale, vec2 origin, float rotation,
			vec2 maskPosition, vec2 maskScale, vec2 maskOrigin, float maskRotation, const vec4& maskSourceRegion);
		void SetColors(const vec4& color);
		void SetColorArray(const vec4 colors[4]);

#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
	public:
		void SetTextureIndices(u32 textureIndex);
#endif
	};

	struct SpriteBatch
	{
		u16 Index;
		u16 Count;

#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
		std::array<const D3D11::Texture2D*, SpriteTextureSlots> Textures = {};
#endif

		SpriteBatch(u16 index, u16 count) : Index(index), Count(count) {};
	};

	struct SpriteBatchItem
	{
		const D3D11::Texture2D* Texture;
		const D3D11::Texture2D* MaskTexture;
		Graphics::AetBlendMode BlendMode;
		bool DrawTextBorder;
		vec2 CheckerboardSize;
	};

	struct SpriteBatchPair
	{
		SpriteBatchItem* Item;
		SpriteVertices* Vertices;
	};
}
