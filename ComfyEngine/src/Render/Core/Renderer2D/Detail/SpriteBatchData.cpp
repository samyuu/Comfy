#include "SpriteBatchData.h"

namespace Comfy::Render::Detail
{
	namespace
	{
		constexpr u32 FloatToUInt32Sat(float value)
		{
			return static_cast<u32>(((value < 0.0f) ? 0.0f : (value > 1.0f) ? 1.0f : value) * 255.0f + 0.5f);
		};

		constexpr u32 Vec4ToUInt32(const vec4& value)
		{
			return ((FloatToUInt32Sat(value.x)) << 0) | ((FloatToUInt32Sat(value.y)) << 8) | ((FloatToUInt32Sat(value.z)) << 16) | ((FloatToUInt32Sat(value.w)) << 24);
		}

		constexpr void RotateVec2(vec2& point, float sin, float cos)
		{
			point = vec2(point.x * cos - point.y * sin, point.x * sin + point.y * cos);
		}
	}

	void SpriteVertices::SetTextureIndices(u32 textureIndex)
	{
		TopLeft.TextureIndex = textureIndex;
		TopRight.TextureIndex = textureIndex;
		BottomLeft.TextureIndex = textureIndex;
		BottomRight.TextureIndex = textureIndex;
	}

	void SpriteVertices::SetValues(vec2 position, const vec4& sourceRegion, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4 colors[4], bool setTexCoords)
	{
		SetPositions(position, vec2(sourceRegion.z, sourceRegion.w) * scale, origin * scale, rotation);

		if (setTexCoords)
		{
			const auto topLeft = vec2(sourceRegion.x / size.x, sourceRegion.y / size.y);
			const auto bottomRight = vec2((sourceRegion.x + sourceRegion.z) / size.x, (sourceRegion.y + sourceRegion.w) / size.y);
			SetTexCoords(topLeft, bottomRight);
		}
		else
		{
			SetNullTexCoords();
		}

		SetColorArray(colors);
	}

	void SpriteVertices::SetPositionsNoRotation(vec2 position, vec2 size)
	{
		TopLeft.Position.x = position.x;
		TopLeft.Position.y = position.y;

		TopRight.Position.x = position.x + size.x;
		TopRight.Position.y = position.y;

		BottomLeft.Position.x = position.x;
		BottomLeft.Position.y = position.y + size.y;

		BottomRight.Position.x = position.x + size.x;
		BottomRight.Position.y = position.y + size.y;
	}

	void SpriteVertices::SetPositions(vec2 position, vec2 size, vec2 origin, float rotation)
	{
		if (rotation == 0.0f)
		{
			SetPositionsNoRotation(position + origin, size);
		}
		else
		{
			const float radians = glm::radians(rotation);
			const float sin = glm::sin(radians);
			const float cos = glm::cos(radians);

			TopLeft.Position.x = position.x + origin.x * cos - origin.y * sin;
			TopLeft.Position.y = position.y + origin.x * sin + origin.y * cos;

			TopRight.Position.x = position.x + (origin.x + size.x) * cos - origin.y * sin;
			TopRight.Position.y = position.y + (origin.x + size.x) * sin + origin.y * cos;

			BottomLeft.Position.x = position.x + origin.x * cos - (origin.y + size.y) * sin;
			BottomLeft.Position.y = position.y + origin.x * sin + (origin.y + size.y) * cos;

			BottomRight.Position.x = position.x + (origin.x + size.x) * cos - (origin.y + size.y) * sin;
			BottomRight.Position.y = position.y + (origin.x + size.x) * sin + (origin.y + size.y) * cos;
		}
	}

	void SpriteVertices::SetTexCoords(vec2 topLeft, vec2 bottomRight)
	{
		TopLeft.TextureCoordinates.x = topLeft.x;
		TopLeft.TextureCoordinates.y = topLeft.y;

		TopRight.TextureCoordinates.x = bottomRight.x;
		TopRight.TextureCoordinates.y = topLeft.y;

		BottomLeft.TextureCoordinates.x = topLeft.x;
		BottomLeft.TextureCoordinates.y = bottomRight.y;

		BottomRight.TextureCoordinates.x = bottomRight.x;
		BottomRight.TextureCoordinates.y = bottomRight.y;
	}

	void SpriteVertices::SetNullTexCoords()
	{
		constexpr auto center = vec2(0.5f, 0.5f);

		TopLeft.TextureCoordinates = center;
		TopRight.TextureCoordinates = center;
		BottomLeft.TextureCoordinates = center;
		BottomRight.TextureCoordinates = center;
	}

	void SpriteVertices::SetTexMaskCoords(
		const D3D11::Texture2D* texture, vec2 position, vec2 scale, vec2 origin, float rotation,
		vec2 maskPosition, vec2 maskScale, vec2 maskOrigin, float maskRotation, const vec4& maskSourceRegion)
	{
		const vec2 maskOffset = maskPosition - (maskOrigin * maskScale);
		const vec2 maskRectSize = vec2(maskScale.x * maskSourceRegion.z, maskScale.y * maskSourceRegion.w);

		TopLeft.TextureMaskCoordinates = vec2(0.0f, 0.0f) + maskOffset;
		TopRight.TextureMaskCoordinates = vec2(maskRectSize.x, 0.0f) + maskOffset;
		BottomLeft.TextureMaskCoordinates = vec2(0.0f, maskRectSize.y) + maskOffset;
		BottomRight.TextureMaskCoordinates = vec2(maskRectSize.x, maskRectSize.y) + maskOffset;

		const vec2 scaledOrigin = (origin * scale);
		const float rotationDifference = maskRotation - rotation;

		if (rotationDifference != 0.0f)
		{
			// BUG: maskRotation isn't handled correctly when the position changes

			const float radians = glm::radians(rotationDifference);
			const float sin = glm::sin(radians);
			const float cos = glm::cos(radians);

			TopLeft.TextureMaskCoordinates -= position;
			TopRight.TextureMaskCoordinates -= position;
			BottomLeft.TextureMaskCoordinates -= position;
			BottomRight.TextureMaskCoordinates -= position;

			RotateVec2(TopLeft.TextureMaskCoordinates, sin, cos);
			RotateVec2(TopRight.TextureMaskCoordinates, sin, cos);
			RotateVec2(BottomLeft.TextureMaskCoordinates, sin, cos);
			RotateVec2(BottomRight.TextureMaskCoordinates, sin, cos);

			TopLeft.TextureMaskCoordinates += scaledOrigin;
			TopRight.TextureMaskCoordinates += scaledOrigin;
			BottomLeft.TextureMaskCoordinates += scaledOrigin;
			BottomRight.TextureMaskCoordinates += scaledOrigin;
		}
		else
		{
			const vec2 positionOffset = position - scaledOrigin;
			TopLeft.TextureMaskCoordinates -= positionOffset;
			TopRight.TextureMaskCoordinates -= positionOffset;
			BottomLeft.TextureMaskCoordinates -= positionOffset;
			BottomRight.TextureMaskCoordinates -= positionOffset;
		}

		const vec2 scaledTextureSize = vec2(texture->GetSize()) * scale;
		TopLeft.TextureMaskCoordinates /= scaledTextureSize;
		TopRight.TextureMaskCoordinates /= scaledTextureSize;
		BottomLeft.TextureMaskCoordinates /= scaledTextureSize;
		BottomRight.TextureMaskCoordinates /= scaledTextureSize;
	}

	void SpriteVertices::SetColors(const vec4& color)
	{
		const u32 packedColor = Vec4ToUInt32(color);
		TopLeft.Color = packedColor;
		TopRight.Color = packedColor;
		BottomLeft.Color = packedColor;
		BottomRight.Color = packedColor;
	}

	void SpriteVertices::SetColorArray(const vec4 colors[4])
	{
		TopLeft.Color = Vec4ToUInt32(colors[0]);
		TopRight.Color = Vec4ToUInt32(colors[1]);
		BottomLeft.Color = Vec4ToUInt32(colors[2]);
		BottomRight.Color = Vec4ToUInt32(colors[3]);
	}
}
