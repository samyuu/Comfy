#pragma once
#include "Types.h"
#include "Graphics/VertexLayouts.h"
#include "Graphics/OpenGL/GL_Texture2D.h"

namespace Graphics
{
	struct SpriteIndices
	{
		uint16_t TopLeft;
		uint16_t BottomLeft;
		uint16_t BottomRight;
		uint16_t BottomRightCopy;
		uint16_t TopRight;
		uint16_t TopLeftCopy;

	public:
		static inline constexpr uint32_t GetIndexCount() { return sizeof(SpriteIndices) / sizeof(uint16_t); };
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
		static inline constexpr uint32_t GetVertexCount() { return sizeof(SpriteVertices) / sizeof(SpriteVertex); };

	public:
		void SetPositions(const vec2& position, const vec2& size);
		void SetPositions(const vec2& position, const vec2& size, const vec2& origin, float rotation);
		void SetTexCoords(const vec2& topLeft, const vec2& bottomRight);
		void SetTexMaskCoords(
			const GL_Texture2D* texture, const vec2& position, const vec2& scale, const vec2& origin, float rotation,
			const vec2& maskPosition, const vec2& maskScale, const vec2& maskOrigin, float maskRotation, const vec4& maskSourceRegion);
		void SetColors(const vec4& color);
		void SetColorArray(const vec4 colors[4]);
	};

	struct SpriteBatch
	{
		uint16_t Index;
		uint16_t Count;

		SpriteBatch(uint16_t index, uint16_t count) : Index(index), Count(count) {};
	};

	struct SpriteBatchItem
	{
		const GL_Texture2D* Texture;
		const GL_Texture2D* MaskTexture;
		AetBlendMode BlendMode;
		vec2 CheckerboardSize;

		void SetValues(const GL_Texture2D* texture, const GL_Texture2D* alphaMask = nullptr, AetBlendMode blendMode = AetBlendMode::Normal);
	};

	struct SpriteBatchPair
	{
		SpriteBatchItem* Item;
		SpriteVertices* Vertices;
	};

}