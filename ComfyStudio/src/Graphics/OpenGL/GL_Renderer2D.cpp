#include "GL_Renderer2D.h"
#include "Graphics/RenderCommand.h"
#include <glm/trigonometric.hpp>

namespace Graphics
{
	namespace
	{
		constexpr vec2 DefaultPosition = { 0.0f, 0.0f };
		constexpr vec2 DefaultOrigin = { 0.0f, 0.0f };
		constexpr vec2 DefaultScale = { 1.0f, 1.0f };
		constexpr vec4 DefaultColor = { 1.0f, 1.0f, 1.0f, 1.0f };
		constexpr float DefaultRotation = 0.0f;

		inline const vec2& PositionOrDefault(const vec2* position)
		{
			return position == nullptr ? DefaultPosition : *position;
		}

		inline const vec4 SourceOrDefault(const vec4* source, const GL_Texture2D* texture)
		{
			return source == nullptr ? vec4(0.0f, 0.0f, texture->GetWidth(), texture->GetHeight()) : *source;
		}

		inline const vec2 SizeOrDefault(const GL_Texture2D* texture, const vec4* source)
		{
			return texture != nullptr ? texture->GetSize() : vec2(source->w, source->z);
		}

		inline const vec2 OriginOrDefault(const vec2* origin)
		{
			return origin == nullptr ? DefaultOrigin : -*origin;
		}

		inline const vec2& ScaleOrDefault(const vec2* scale)
		{
			return scale == nullptr ? DefaultScale : *scale;
		}

		inline const vec4& ColorOrDefault(const vec4* color)
		{
			return color == nullptr ? DefaultColor : *color;
		}
	}

	void GL_Renderer2D::Initialize()
	{
		const BufferLayout layout =
		{
			{ ShaderDataType::Vec2,		 "in_Position" },
			{ ShaderDataType::Vec2,		 "in_TextureCoords" },
			{ ShaderDataType::Vec2,		 "in_TextureMaskCoords" },
			{ ShaderDataType::vec4_Byte, "in_Color", true },
		};

		indexBuffer.InitializeID();
		indexBuffer.Bind();
		indexBuffer.SetObjectLabel("Renderer2D::IndexBuffer");
		GenerateUploadSpriteIndexBuffer();

		vertexBuffer.InitializeID();
		vertexBuffer.Bind();
		vertexBuffer.SetObjectLabel("Renderer2D::VertexBuffer");

		spriteShader = MakeUnique<GL_SpriteShader>();
		spriteShader->Initialize();

		vertexArray.InitializeID();
		vertexArray.Bind();
		vertexArray.SetLayout(layout);
		vertexArray.SetObjectLabel("Renderer2D::VertexArray");

		batches.reserve(MaxBatchItemSize);
		batchItems.reserve(MaxBatchItemSize);
		vertices.reserve(MaxBatchItemSize);
	}

	void GL_Renderer2D::Begin(const OrthographicCamera& camera)
	{
		drawCallCount = 0;
		orthographicCamera = &camera;
	}

	void GL_Renderer2D::Draw(const vec2& position, const vec2& size, const vec4& color)
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		DrawInternal(nullptr, &source, &position, nullptr, DefaultRotation, nullptr, &color);
	}

	void GL_Renderer2D::Draw(const vec2& position, const vec2& size, const vec4 colors[4])
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);

		SpriteBatchPair pair = CheckFlushAddItem();
		pair.Item->SetValues(nullptr, nullptr);
		pair.Vertices->SetValues(position, source, size, DefaultOrigin, DefaultRotation, DefaultScale, colors);
	}

	void GL_Renderer2D::Draw(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color)
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		DrawInternal(nullptr, &source, &position, &origin, rotation, &scale, &color);
	}

	void GL_Renderer2D::Draw(const GL_Texture2D* texture, const vec2& position, const vec4& color)
	{
		DrawInternal(texture, nullptr, &position, nullptr, DefaultRotation, nullptr, &color);
	}

	void GL_Renderer2D::Draw(const GL_Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec4& color)
	{
		DrawInternal(texture, &sourceRegion, &position, nullptr, 0.0f, nullptr, &color);
	}

	void GL_Renderer2D::Draw(const GL_Texture2D* texture, const vec2& position, const vec2& origin, float rotation, const vec4& color)
	{
		DrawInternal(texture, nullptr, &position, &origin, rotation, nullptr, &color);
	}

	void GL_Renderer2D::Draw(const GL_Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color, AetBlendMode blendMode)
	{
		DrawInternal(texture, &sourceRegion, &position, &origin, rotation, &scale, &color, blendMode);
	}

	void GL_Renderer2D::Draw(
		const GL_Texture2D* maskTexture, const vec4& maskSourceRegion, const vec2& maskPosition, const vec2& maskOrigin, float maskRotation, const vec2& maskScale,
		const GL_Texture2D* texture, const vec4& sourceRegion, const vec2& position, const vec2& origin, float rotation, const vec2& scale, const vec4& color,
		AetBlendMode blendMode)
	{
		SpriteBatchPair pair = CheckFlushAddItem();

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

	void GL_Renderer2D::DrawLine(const vec2& start, const vec2& end, const vec4& color, float thickness)
	{
		const vec2 edge = end - start;

		const float distance = glm::distance(start, end);
		const float angle = glm::degrees(glm::atan(edge.y, edge.x));

		const vec4 source = vec4(0.0f, 0.0f, distance, thickness);
		const vec2 origin = vec2(0.0f, thickness / 2.0f);

		DrawInternal(nullptr, &source, &start, &origin, angle, nullptr, &color);
	}

	void GL_Renderer2D::DrawLine(const vec2& start, float angle, float length, const vec4& color, float thickness)
	{
		const vec4 source = vec4(0.0f, 0.0f, length, thickness);
		const vec2 origin = vec2(0.0f, thickness / 2.0f);

		DrawInternal(nullptr, &source, &start, &origin, angle, nullptr, &color);
	}

	void GL_Renderer2D::DrawRectangle(const vec2& topLeft, const vec2& topRight, const vec2& bottomLeft, const vec2& bottomRight, const vec4& color, float thickness)
	{
		DrawLine(topLeft, topRight, color, thickness);
		DrawLine(topRight, bottomRight, color, thickness);
		DrawLine(bottomRight, bottomLeft, color, thickness);
		DrawLine(bottomLeft, topLeft, color, thickness);
	}

	void GL_Renderer2D::DrawCheckerboardRectangle(const vec2& position, const vec2& size, const vec2& origin, float rotation, const vec2& scale, const vec4& color, float precision)
	{
		const vec4 source = vec4(0.0f, 0.0f, size);
		DrawInternal(nullptr, &source, &position, &origin, rotation, &scale, &color);

		batchItems.back().CheckerboardSize = size * scale * precision;
	}

	const SpriteVertices& GL_Renderer2D::GetLastVertices() const
	{
		assert(vertices.size() > 0);
		return vertices.back();
	}

	void GL_Renderer2D::End()
	{
		Flush();
		orthographicCamera = nullptr;
	}

	void GL_Renderer2D::Flush()
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
		spriteShader->SetUniform(spriteShader->ProjectionView, orthographicCamera->GetProjectionMatrix() * orthographicCamera->GetViewMatrix());

		vertexArray.Bind();
		vertexBuffer.Bind();
		vertexBuffer.Upload(vertices.size() * sizeof(SpriteVertices), vertices.data());
		indexBuffer.Bind();

		AetBlendMode lastBlendMode;

		for (uint16_t i = 0; i < batches.size(); i++)
		{
			SpriteBatch& batch = batches[i];
			SpriteBatchItem& item = batchItems[batch.Index];

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

	void GL_Renderer2D::SetBlendFunction(AetBlendMode blendMode)
	{
		const GL_BlendFuncStruct blendFunc = GetBlendFuncParamteres(blendMode);
		GLCall(glBlendFuncSeparate(
			blendFunc.SourceRGB,
			blendFunc.DestinationRGB,
			blendFunc.SourceAlpha,
			blendFunc.DestinationAlpha));
	}

	const GL_BlendFuncStruct GL_Renderer2D::GetBlendFuncParamteres(AetBlendMode blendMode)
	{
		switch (blendMode)
		{
		default:
		case AetBlendMode::Normal:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE };
		case AetBlendMode::Add:
			return { GL_SRC_ALPHA, GL_ONE, GL_ZERO, GL_ONE };
		case AetBlendMode::Multiply:
			return { GL_DST_COLOR, GL_ZERO, GL_ZERO, GL_ONE };
		case AetBlendMode::LinearDodge:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_COLOR, GL_ZERO, GL_ONE };
		case AetBlendMode::Overlay:
			return { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE };
		}

		return { GL_INVALID_ENUM, GL_INVALID_ENUM, GL_INVALID_ENUM, GL_INVALID_ENUM };
	}

	void GL_Renderer2D::GenerateUploadSpriteIndexBuffer()
	{
		std::array<SpriteIndices, MaxBatchItemSize> indexData;
		for (uint16_t i = 0, offset = 0; i < indexData.size(); i++)
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

		indexBuffer.Upload(sizeof(SpriteIndices) * indexData.size(), indexData.data());
	}

	void GL_Renderer2D::CreateBatches()
	{
		const GL_Texture2D* lastTexture = nullptr;

		for (uint16_t i = 0; i < batchItems.size(); i++)
		{
			bool first = i == 0;

			SpriteBatchItem* item = &batchItems[i];
			SpriteBatchItem* lastItem = first ? nullptr : &batchItems[batches.back().Index];

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

	void GL_Renderer2D::CheckFlushItems()
	{
		if (batchItems.size() >= MaxBatchItemSize)
			Flush();
	}

	SpriteBatchPair GL_Renderer2D::CheckFlushAddItem()
	{
		CheckFlushItems();
		return AddItem();
	}

	SpriteBatchPair GL_Renderer2D::AddItem()
	{
		batchItems.emplace_back();
		vertices.emplace_back();

		return { &batchItems.back(), &vertices.back() };
	}

	void GL_Renderer2D::ClearItems()
	{
		batches.clear();
		batchItems.clear();
		vertices.clear();
	}

	void GL_Renderer2D::DrawInternal(const GL_Texture2D* texture, const vec4* sourceRegion, const vec2* position, const vec2* origin, float rotation, const vec2* scale, const vec4* color, AetBlendMode blendMode)
	{
		SpriteBatchPair pair = CheckFlushAddItem();

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
