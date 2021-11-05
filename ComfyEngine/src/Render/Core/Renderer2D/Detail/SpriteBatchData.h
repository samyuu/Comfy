#pragma once
#include "Types.h"
#include "Render/Core/Renderer2D/RenderCommand2D.h"
#include "Render/D3D11/D3D11Texture.h"
#include <optional>

namespace Comfy::Render
{
	constexpr size_t MaxSpriteTextureSlots = 8;
}

namespace Comfy::Render::Detail
{
	constexpr u32 FloatToUInt32Sat(float value)
	{
		return static_cast<u32>(((value < 0.0f) ? 0.0f : (value > 1.0f) ? 1.0f : value) * 255.0f + 0.5f);
	};

	constexpr u32 ColorVec4ToU32(const vec4& value)
	{
		return ((FloatToUInt32Sat(value.x)) << 0) | ((FloatToUInt32Sat(value.y)) << 8) | ((FloatToUInt32Sat(value.z)) << 16) | ((FloatToUInt32Sat(value.w)) << 24);
	}

	struct SpriteQuadIndices
	{
		u16 TopLeft;
		u16 BottomLeft;
		u16 BottomRight;
		u16 BottomRightCopy;
		u16 TopRight;
		u16 TopLeftCopy;

		static constexpr u32 TotalIndices() { return sizeof(SpriteQuadIndices) / sizeof(u16); };
	};

	struct SpriteVertex
	{
		vec2 Position;
		vec2 TextureCoordinates;
		vec2 TextureMaskCoordinates;
		u32 Color;
		u32 TextureIndex;
	};

	struct SpriteQuadVertices
	{
		SpriteVertex TopLeft;
		SpriteVertex TopRight;
		SpriteVertex BottomLeft;
		SpriteVertex BottomRight;

	public:
		void SetValues(vec2 position, const vec4& sourceRegion, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4 colors[4], bool setTexCoords, bool flipTexY);
		static constexpr u32 GetVertexCount() { return sizeof(SpriteQuadVertices) / sizeof(SpriteVertex); };

	public:
		void SetPositionsNoRotation(vec2 position, vec2 size);
		void SetPositions(vec2 position, vec2 size, vec2 origin, float rotation);
		void SetTexCoords(vec2 topLeft, vec2 bottomRight, bool flipY);
		void SetCenterTexCoords();
		void SetTexMaskCoords(
			TexSamplerView texView, vec2 position, vec2 scale, vec2 origin, float rotation,
			vec2 maskPosition, vec2 maskScale, vec2 maskOrigin, float maskRotation, const vec4& maskSourceRegion);
		void SetColors(const vec4& color);
		void SetColorArray(const vec4 colors[4]);

	public:
		void SetTextureIndices(u32 textureIndex);
	};

	struct SpriteDrawCallBatch
	{
		SpriteDrawCallBatch(u16 index, u16 count, u16 quadIndex)
			: ItemIndex(index), ItemCount(count), QuadIndex(quadIndex)
		{
		}

		u16 ItemIndex;
		u16 ItemCount;
		u16 QuadIndex;

		std::array<TexSamplerView, MaxSpriteTextureSlots> TexViews = {};
	};

	struct SpriteBatchItem
	{
		// NOTE: Never nullptr (WhiteChipTexture used as a backup)
		TexSamplerView TexView;

		// NOTE: May be nullptr
		TexSamplerView MaskTexView;

		Graphics::PrimitiveType Primitive;
		Graphics::AetBlendMode BlendMode;

		bool DrawTextBorder;
		bool DrawCheckerboard;

		vec2 CheckerboardSize;

		u16 ShapeVertexIndex;
		u16 ShapeVertexCount;
	};

	struct SpriteBatchItemVertexView
	{
		SpriteBatchItem* Item;
		SpriteQuadVertices* Vertices;
	};
}
