#include "Renderer2D.h"
#include "Graphics/RenderCommand.h"
#include <glm/trigonometric.hpp>

namespace Graphics::Auth2D
{
	static inline uint32_t FloatToUInt32Sat(float value)
	{
		return static_cast<uint32_t>(((value < 0.0f) ? 0.0f : (value > 1.0f) ? 1.0f : value) * 255.0f + 0.5f);
	};

	static inline uint32_t Vec4ToUInt32(const vec4& value)
	{
		return ((FloatToUInt32Sat(value.x)) << 0) | ((FloatToUInt32Sat(value.y)) << 8) | ((FloatToUInt32Sat(value.z)) << 16) | ((FloatToUInt32Sat(value.w)) << 24);
	}

	constexpr vec2 DefaultPosition = { 0.0f, 0.0f };
	constexpr vec2 DefaultOrigin = { 0.0f, 0.0f };
	constexpr vec2 DefaultScale = { 1.0f, 1.0f };
	constexpr vec4 DefaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	constexpr float DefaultRotation = 0.0f;

	static inline const vec2& PositionOrDefault(const vec2* position)
	{
		return position == nullptr ? DefaultPosition : *position;
	}

	static inline const vec4 SourceOrDefault(const vec4* source, const Texture2D* texture)
	{
		return source == nullptr ? vec4(0.0f, 0.0f, texture->GetWidth(), texture->GetHeight()) : *source;
	}

	static inline const vec2 SizeOrDefault(const Texture2D* texture, const vec4* source)
	{
		return texture != nullptr ? texture->GetSize() : vec2(source->w, source->z);
	}

	static inline const vec2 OriginOrDefault(const vec2* origin)
	{
		return origin == nullptr ? DefaultOrigin : -*origin;
	}

	static inline const vec2& ScaleOrDefault(const vec2* scale)
	{
		return scale == nullptr ? DefaultScale : *scale;
	}

	static inline const vec4& ColorOrDefault(const vec4* color)
	{
		return color == nullptr ? DefaultColor : *color;
	}

	static inline void RotateVector(vec2& point, float sin, float cos)
	{
		point = vec2(point.x * cos - point.y * sin, point.x * sin + point.y * cos);
	}

	void SpriteVertices::SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color)
	{
		SetPositions(position, vec2(sourceRegion.z, sourceRegion.w) * scale, origin * scale, rotation);

		vec2 topLeft = vec2(sourceRegion.x / size.x, sourceRegion.y / size.y);
		vec2 bottomRight = vec2((sourceRegion.x + sourceRegion.z) / size.x, (sourceRegion.y + sourceRegion.w) / size.y);

		SetTexCoords(topLeft, bottomRight);
		SetColors(color);
	}

	void SpriteVertices::SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4 colors[4])
	{
		SetPositions(position, vec2(sourceRegion.z, sourceRegion.w) * scale, origin * scale, rotation);

		vec2 topLeft = vec2(sourceRegion.x / size.x, sourceRegion.y / size.y);
		vec2 bottomRight = vec2((sourceRegion.x + sourceRegion.z) / size.x, (sourceRegion.y + sourceRegion.w) / size.y);

		SetTexCoords(topLeft, bottomRight);
		SetColorArray(colors);
	}

	void SpriteVertices::SetPositions(const vec2& position, const vec2& size)
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

	void SpriteVertices::SetPositions(const vec2& position, const vec2& size, const vec2& origin, float rotation)
	{
		if (rotation == 0.0f)
		{
			SetPositions(position + origin, size);
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

	void SpriteVertices::SetTexCoords(const vec2& topLeft, const vec2& bottomRight)
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

	void SpriteVertices::SetTexMaskCoords(
		const Texture2D* texture, const vec2& position, const vec2& scale, const vec2& origin, float rotation,
		const vec2& maskPosition, const vec2& maskScale, const vec2& maskOrigin, float maskRotation, const vec4& maskSourceRegion)
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

			RotateVector(TopLeft.TextureMaskCoordinates, sin, cos);
			RotateVector(TopRight.TextureMaskCoordinates, sin, cos);
			RotateVector(BottomLeft.TextureMaskCoordinates, sin, cos);
			RotateVector(BottomRight.TextureMaskCoordinates, sin, cos);

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

		const vec2 scaledTextureSize = texture->GetSize() * scale;
		TopLeft.TextureMaskCoordinates /= scaledTextureSize;
		TopRight.TextureMaskCoordinates /= scaledTextureSize;
		BottomLeft.TextureMaskCoordinates /= scaledTextureSize;
		BottomRight.TextureMaskCoordinates /= scaledTextureSize;
	}

	void SpriteVertices::SetColors(const vec4& color)
	{
		uint32_t packedColor = Vec4ToUInt32(color);
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

	void BatchItem::SetValues(const Texture2D* texture, const Texture2D* alphaMask, AetBlendMode blendMode)
	{
		Texture = texture;
		MaskTexture = alphaMask;
		BlendMode = blendMode;
	}

	void Renderer2D::Initialize()
	{
		BufferLayout layout =
		{
			{ ShaderDataType::Vec2,		 "in_Position" },
			{ ShaderDataType::Vec2,		 "in_TextureCoords" },
			{ ShaderDataType::Vec2,		 "in_TextureMaskCoords" },
			{ ShaderDataType::vec4_Byte, "in_Color", true },
		};

		indexBuffer.InitializeID();
		indexBuffer.Bind();
		indexBuffer.SetObjectLabel("Renderer2D::IndexBuffer");
		GenerateUploadSpriteIndexBuffer(Renderer2DMaxItemSize);

		vertexBuffer.InitializeID();
		vertexBuffer.Bind();
		vertexBuffer.SetObjectLabel("Renderer2D::VertexBuffer");

		spriteShader = MakeUnique<SpriteShader>();
		spriteShader->Initialize();

		vertexArray.InitializeID();
		vertexArray.Bind();
		vertexArray.SetLayout(layout);
		vertexArray.SetObjectLabel("Renderer2D::VertexArray");

		batches.reserve(Renderer2DMaxItemSize);
		batchItems.reserve(Renderer2DMaxItemSize);
		vertices.reserve(Renderer2DMaxItemSize);
	}

	void Renderer2D::Begin(const OrthographicCamera& camera)
	{
		this->drawCallCount = 0;
		this->camera = &camera;
	}

	void Renderer2D::Draw(const vec2& position, const vec2& size, const vec4& color)
	{
		vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		DrawInternal(nullptr, &source, &position, nullptr, DefaultRotation, nullptr, &color);
	}

	void Renderer2D::Draw(const vec2& position, const vec2& size, const vec4 colors[4])
	{
		vec4 source = vec4(0.0f, 0.0f, size.x, size.y);

		BatchPair pair = CheckFlushAddItem();
		pair.Item->SetValues(nullptr, nullptr);
		pair.Vertices->SetValues(position, source, size, DefaultOrigin, DefaultRotation, DefaultScale, colors);
	}

	void Renderer2D::Draw(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color)
	{
		vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		DrawInternal(nullptr, &source, &position, &origin, rotation, &scale, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, const vec2& position, const vec4& color)
	{
		DrawInternal(texture, nullptr, &position, nullptr, DefaultRotation, nullptr, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec4& color)
	{
		DrawInternal(texture, &sourceRegion, &position, nullptr, 0.0f, nullptr, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, const vec2& position, const vec2& origin, float rotation, const vec4& color)
	{
		DrawInternal(texture, nullptr, &position, &origin, rotation, nullptr, &color);
	}

	void Renderer2D::Draw(const Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color, AetBlendMode blendMode)
	{
		DrawInternal(texture, &sourceRegion, &position, &origin, rotation, &scale, &color, blendMode);
	}

	void Renderer2D::Draw(
		const Texture2D* maskTexture, const vec4& maskSourceRegion, const vec2& maskPosition, const vec2& maskOrigin, float maskRotation, const vec2& maskScale,
		const Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color,
		AetBlendMode blendMode)
	{
		BatchPair pair = CheckFlushAddItem();

		pair.Item->SetValues(
			texture,
			maskTexture,
			blendMode);

		pair.Vertices->SetValues(
			maskPosition,
			maskSourceRegion,
			maskTexture->GetSize(),
			-maskOrigin,
			maskRotation,
			maskScale,
			color);

		pair.Vertices->SetTexMaskCoords(
			texture,
			position,
			scale,
			origin + vec2(sourceRegion.x, sourceRegion.y),
			rotation,
			maskPosition,
			maskScale,
			maskOrigin,
			maskRotation,
			maskSourceRegion);
	}

	void Renderer2D::DrawLine(const vec2& start, const vec2& end, const vec4& color, float thickness)
	{
		vec2 edge = end - start;

		float distance = glm::distance(start, end);
		float angle = glm::degrees(glm::atan(edge.y, edge.x));

		vec4 source = vec4(0.0f, 0.0f, distance, thickness);
		vec2 origin = vec2(0.0f, thickness / 2.0f);

		DrawInternal(nullptr, &source, &start, &origin, angle, nullptr, &color);
	}

	void Renderer2D::DrawLine(const vec2& start, float angle, float length, const vec4& color, float thickness)
	{
		vec4 source = vec4(0.0f, 0.0f, length, thickness);
		vec2 origin = vec2(0.0f, thickness / 2.0f);

		DrawInternal(nullptr, &source, &start, &origin, angle, nullptr, &color);
	}

	void Renderer2D::DrawRectangle(const vec2& topLeft, const vec2& topRight, const vec2& bottomLeft, const vec2& bottomRight, const vec4& color, float thickness)
	{
		DrawLine(topLeft, topRight, color, thickness);
		DrawLine(topRight, bottomRight, color, thickness);
		DrawLine(bottomRight, bottomLeft, color, thickness);
		DrawLine(bottomLeft, topLeft, color, thickness);
	}

	void Renderer2D::DrawCheckerboardRectangle(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color, float precision)
	{
		vec4 source = vec4(0.0f, 0.0f, size);
		DrawInternal(nullptr, &source, &position, &origin, rotation, &scale, &color);

		batchItems.back().CheckerboardSize = size * scale * precision;
	}

	const SpriteVertices& Renderer2D::GetLastVertices() const
	{
		assert(vertices.size() > 0);
		return vertices.back();
	}

	void Renderer2D::End()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		CreateBatches();

		GLCall(glDisable(GL_DEPTH_TEST));

		if (enableAlphaTest)
		{
			GLCall(glEnable(GL_BLEND));
		}
		else
		{
			GLCall(glDisable(GL_BLEND));
		}

		enum SpriteShaderTextureSlot { TextureSpriteSlot = 0, TextureMaskSlot = 1 };

		spriteShader->Bind();
		spriteShader->SetUniform(spriteShader->Texture, TextureSpriteSlot);
		spriteShader->SetUniform(spriteShader->TextureMask, TextureMaskSlot);
		spriteShader->SetUniform(spriteShader->UseTextShadow, GetUseTextShadow());
		spriteShader->SetUniform(spriteShader->ProjectionView, camera->GetProjectionMatrix() * camera->GetViewMatrix());

		vertexArray.Bind();
		vertexBuffer.Bind();
		vertexBuffer.Upload(vertices.size() * sizeof(SpriteVertices), vertices.data());
		indexBuffer.Bind();

		AetBlendMode lastBlendMode;

		for (uint16_t i = 0; i < batches.size(); i++)
		{
			Batch& batch = batches[i];
			BatchItem& item = batchItems[batch.Index];

			bool firstItem = i == 0;
			if (firstItem || lastBlendMode != item.BlendMode)
			{
				lastBlendMode = item.BlendMode;
				SetBlendFunction(lastBlendMode);
			}

			spriteShader->SetUniform(spriteShader->TextureMaskFormat, item.MaskTexture != nullptr ? static_cast<int>(item.MaskTexture->GetTextureFormat()) : -1);
			if (item.MaskTexture != nullptr)
			{
				item.MaskTexture->Bind(static_cast<TextureSlot>(TextureMaskSlot));
			}

			if (item.Texture != nullptr)
			{
				item.Texture->Bind(static_cast<TextureSlot>(TextureSpriteSlot));

				if (item.Texture == item.MaskTexture)
				{
					// NOTE: Special case for when the texture mask shares the same texture
					spriteShader->SetUniform(spriteShader->TextureFormat, -1);
				}
				else
				{
					spriteShader->SetUniform(spriteShader->TextureFormat, static_cast<int>(item.Texture->GetTextureFormat()));
				}
			}

			spriteShader->SetUniform(spriteShader->UseSolidColor, item.Texture == nullptr);

			bool useCheckerboard = item.CheckerboardSize != vec2(0.0f);
			spriteShader->SetUniform(spriteShader->UseCheckerboard, useCheckerboard);
			if (useCheckerboard)
				spriteShader->SetUniform(spriteShader->CheckerboardSize, item.CheckerboardSize);

			RenderCommand::DrawElements(PrimitiveType::Triangles,
				batch.Count * SpriteIndices::GetIndexCount(),
				indexBuffer.GetGLIndexType(),
				reinterpret_cast<void*>(batch.Index * sizeof(SpriteIndices)));

			drawCallCount++;
		}

		vertexArray.UnBind();

		ClearItems();
	}

	void Renderer2D::SetBlendFunction(AetBlendMode blendMode)
	{
		const BlendFuncStruct blendFunc = GetBlendFuncParamteres(blendMode);
		GLCall(glBlendFuncSeparate(
			blendFunc.SourceRGB,
			blendFunc.DestinationRGB,
			blendFunc.SourceAlpha,
			blendFunc.DestinationAlpha));
	}

	const BlendFuncStruct Renderer2D::GetBlendFuncParamteres(AetBlendMode blendMode)
	{
		switch (blendMode)
		{
		default:
		case AetBlendMode::Alpha:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE };
		case AetBlendMode::Additive:
			return { GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE };
		case AetBlendMode::DstColorZero:
			return { GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE };
		case AetBlendMode::SrcAlphaOneMinusSrcColor:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE };
		case AetBlendMode::Transparent:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE };
		}

		return { GL_INVALID_ENUM, GL_INVALID_ENUM, GL_INVALID_ENUM, GL_INVALID_ENUM };
	}

	void Renderer2D::GenerateUploadSpriteIndexBuffer(uint16_t elementCount)
	{
		SpriteIndices* indexData = new SpriteIndices[elementCount];
		{
			for (uint16_t i = 0, offset = 0; i < elementCount; i++)
			{
				// [0] TopLeft	  - [1] TopRight
				// [2] BottomLeft - [3] BottomRight;

				indexData[i] =
				{
					static_cast<uint16_t>(offset + 0), // [0] TopLeft;
					static_cast<uint16_t>(offset + 2), // [2] BottomLeft;
					static_cast<uint16_t>(offset + 3), // [3] BottomRight;

					static_cast<uint16_t>(offset + 3), // [3] BottomRight;
					static_cast<uint16_t>(offset + 1), // [1] TopRight;
					static_cast<uint16_t>(offset + 0), // [0] TopLeft;
				};

				offset += SpriteVertices::GetVertexCount();
			}

			indexBuffer.Upload(sizeof(SpriteIndices) * elementCount, indexData);
		}
		delete[] indexData;
	}

	void Renderer2D::CreateBatches()
	{
		const Texture2D* lastTexture = nullptr;

		for (uint16_t i = 0; i < batchItems.size(); i++)
		{
			bool first = i == 0;

			BatchItem* item = &batchItems[i];
			BatchItem* lastItem = first ? nullptr : &batchItems[batches.back().Index];

			constexpr vec2 sizeZero = vec2(0.0f);
			bool newBatch = first ||
				(item->BlendMode != lastItem->BlendMode) ||
				(item->Texture != lastItem->Texture) ||
				(item->CheckerboardSize != sizeZero || lastItem->CheckerboardSize != sizeZero) ||
				(item->MaskTexture != nullptr);

			if (!batchSprites)
				newBatch = true;

			if (newBatch)
			{
				batches.emplace_back(i, 1);
			}
			else
			{
				batches.back().Count++;
			}
		}
	}

	void Renderer2D::CheckFlushItems()
	{
		if (batchItems.size() >= Renderer2DMaxItemSize)
			Flush();
	}

	BatchPair Renderer2D::CheckFlushAddItem()
	{
		CheckFlushItems();
		return AddItem();
	}

	BatchPair Renderer2D::AddItem()
	{
		batchItems.emplace_back();
		vertices.emplace_back();

		return { &batchItems.back(), &vertices.back() };
	}

	void Renderer2D::ClearItems()
	{
		batches.clear();
		batchItems.clear();
		vertices.clear();
	}

	void Renderer2D::DrawInternal(const Texture2D* texture, const vec4* sourceRegion, const vec2* position, const vec2* origin, float rotation, const vec2* scale, const vec4* color, AetBlendMode blendMode)
	{
		BatchPair pair = CheckFlushAddItem();

		pair.Item->SetValues(
			texture,
			nullptr,
			blendMode);

		pair.Vertices->SetValues(
			PositionOrDefault(position),
			SourceOrDefault(sourceRegion, texture),
			SizeOrDefault(texture, sourceRegion),
			OriginOrDefault(origin),
			rotation,
			ScaleOrDefault(scale),
			ColorOrDefault(color));
	}
}
