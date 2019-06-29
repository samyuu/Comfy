#include "Renderer2D.h"

namespace Auth2D
{
	void SpriteVertices::SetValues(const vec2& position, const vec2& size, const vec4& color)
	{
		SetPositions(position, size);
		SetTexCoords(vec2(0, 1), vec2(1, 0));
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

	void Renderer2D::Draw(const vec2& position, const vec2 size, const vec4& color)
	{
		CheckFlushItems();
		BatchPair pair = AddItem();

		pair.Item->SetValues(nullptr);
		pair.Vertices->SetValues(position, size, color);
	}

	void Renderer2D::Draw(const Texture2D* texture, const vec2& position, const vec4& color, AetBlendMode blendMode)
	{
		CheckFlushItems();
		BatchPair pair = AddItem();

		pair.Item->SetValues(texture, nullptr, blendMode);
		pair.Vertices->SetValues(position, texture->GetSize(), color);
	}

	void Renderer2D::End()
	{
		Flush();
	}

	void Renderer2D::Flush()
	{
		CreateBatches();

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);

		vertexArray.Bind();
		vertexBuffer.Bind();
		vertexBuffer.Upload(vertices.size() * sizeof(SpriteVertices), vertices.data());
		shader->Bind();

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

			glDrawElements(GL_TRIANGLES,
				batch.Count * SpriteIndices::GetIndexCount(),
				indexBuffer.GetGLIndexType(),
				(void*)(batch.Index * sizeof(SpriteIndices)));
		}

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
		glBlendFuncSeparate(
			blendFunc.SourceRGB,
			blendFunc.DestinationRGB,
			blendFunc.SourceAlpha,
			blendFunc.DestinationAlpha);
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

					static_cast<uint16_t>(offset + 3), // [3] BottomRightCopy;
					static_cast<uint16_t>(offset + 1), // [1] TopRight;
					static_cast<uint16_t>(offset + 0), // [0] TopLeftCopy;
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
}
