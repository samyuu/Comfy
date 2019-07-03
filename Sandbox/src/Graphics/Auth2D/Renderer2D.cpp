#include "Renderer2D.h"

namespace Auth2D
{
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

	void SpriteVertices::SetValues(const vec2& position, const vec4& sourceRegion, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color)
	{
		SetPositions(position, vec2(sourceRegion.z, sourceRegion.w) * scale, origin * scale, rotation);

		vec2 topLeft = vec2(sourceRegion.x / size.x, sourceRegion.y / size.y);
		vec2 bottomRight = vec2((sourceRegion.x + sourceRegion.z) / size.x, (sourceRegion.y + sourceRegion.w) / size.y);

		SetTexCoords(topLeft, bottomRight);
		SetColors(color);
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
			return;
		}

		float radians = glm::radians(rotation);
		float sin = glm::sin(radians);
		float cos = glm::cos(radians);

		TopLeft.Position.x = position.x + origin.x * cos - origin.y * sin;
		TopLeft.Position.y = position.y + origin.x * sin + origin.y * cos;

		TopRight.Position.x = position.x + (origin.x + size.x) * cos - origin.y * sin;
		TopRight.Position.y = position.y + (origin.x + size.x) * sin + origin.y * cos;

		BottomLeft.Position.x = position.x + origin.x * cos - (origin.y + size.y) * sin;
		BottomLeft.Position.y = position.y + origin.x * sin + (origin.y + size.y) * cos;

		BottomRight.Position.x = position.x + (origin.x + size.x) * cos - (origin.y + size.y) * sin;
		BottomRight.Position.y = position.y + (origin.x + size.x) * sin + (origin.y + size.y) * cos;
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

	void SpriteVertices::SetColors(const vec4& color)
	{
		TopLeft.Color = color;
		TopRight.Color = color;
		BottomLeft.Color = color;
		BottomRight.Color = color;
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
			{ ShaderDataType::Vec2, "in_position" },
			{ ShaderDataType::Vec2, "in_texture_coords" },
			{ ShaderDataType::Vec4, "in_color" }
		};

		indexBuffer.InitializeID();
		indexBuffer.Bind();
		GenerateUploadSpriteIndexBuffer(Renderer2DMaxItemSize);

		vertexBuffer.InitializeID();
		vertexBuffer.Bind();

		shader = std::make_unique<SpriteShader>();
		shader->Initialize();
		shader->Bind();

		vertexArray.InitializeID();
		vertexArray.Bind();
		vertexArray.SetLayout(layout);

		batches.reserve(Renderer2DMaxItemSize);
		batchItems.reserve(Renderer2DMaxItemSize);
		vertices.reserve(Renderer2DMaxItemSize);
	}

	void Renderer2D::Begin()
	{
	}

	void Renderer2D::Draw(const vec2& position, const vec2& size, const vec4& color)
	{
		vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		DrawInternal(nullptr, &source, &position, nullptr, DefaultRotation, nullptr, &color);
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

	void Renderer2D::End()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		CreateBatches();

		GLCall(glDisable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_BLEND));

		shader->Bind();
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

			if (item.Texture != nullptr)
			{
				item.Texture->Bind();
				shader->SetUniform(shader->TextureFormatLocation, static_cast<int>(item.Texture->GetTextureFormat()));
			}

			shader->SetUniform(shader->UseSolidColorLocation, item.Texture == nullptr);

			GLCall(glDrawElements(GL_TRIANGLES,
				batch.Count * SpriteIndices::GetIndexCount(),
				indexBuffer.GetGLIndexType(),
				(void*)(batch.Index * sizeof(SpriteIndices))));
		}

		vertexArray.UnBind();

		ClearItems();
	}

	void Renderer2D::Resize(float width, float height)
	{
		mat4 projection = glm::ortho(0.0f, width, height, 0.0f, -1.0f, 1.0f);

		shader->Bind();
		shader->SetUniform(shader->ProjectionLocation, projection);
	}

	void Renderer2D::SetBlendFunction(AetBlendMode blendMode)
	{
		const BlendFuncStruct& blendFunc = GetBlendFuncParamteres(blendMode);
		GLCall(glBlendFuncSeparate(
			blendFunc.SourceRGB,
			blendFunc.DestinationRGB,
			blendFunc.SourceAlpha,
			blendFunc.DestinationAlpha));
	}

	const BlendFuncStruct& Renderer2D::GetBlendFuncParamteres(AetBlendMode blendMode)
	{
		static BlendFuncStruct lookupTable[6] =
		{
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE },
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE },
			{ GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE },
			{ GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE },
			{ GL_ONE, GL_ZERO, GL_ONE, GL_ZERO },
			{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE },
		};

		int lookupIndex = 0;
		switch (blendMode)
		{
		case AetBlendMode::Alpha:
			lookupIndex = 0; break;
		case AetBlendMode::Additive:
			lookupIndex = 2; break;
		case AetBlendMode::DstColorZero:
			lookupIndex = 3; break;
		case AetBlendMode::SrcAlphaOneMinusSrcColor:
			lookupIndex = 1; break;
		case AetBlendMode::Transparent:
			lookupIndex = 5; break;
		}
		return lookupTable[lookupIndex];
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
		// move to draw (?)
		const Texture2D* lastTexture = nullptr;

		for (uint16_t i = 0; i < batchItems.size(); i++)
		{
			bool first = i == 0;

			BatchItem* item = &batchItems[i];
			BatchItem* lastItem = first ? nullptr : &batchItems[batches.back().Index];

			if (first || (item->BlendMode != lastItem->BlendMode) || (item->Texture != nullptr && (item->Texture != lastItem->Texture || item->MaskTexture != lastItem->MaskTexture)))
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
