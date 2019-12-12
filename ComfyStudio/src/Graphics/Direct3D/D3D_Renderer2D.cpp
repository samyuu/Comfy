#include "D3D_Renderer2D.h"
#include "ShaderBytecode/ShaderBytecode.h"

namespace Graphics
{
	namespace
	{
		struct DefaultProperties
		{
			static constexpr vec2 Position = { 0.0f, 0.0f };
			static constexpr vec2 Origin = { 0.0f, 0.0f };
			static constexpr vec2 Scale = { 1.0f, 1.0f };
			static constexpr vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };
			static constexpr float Rotation = 0.0f;

			static vec2 PositionOrDefault(const vec2* position)
			{
				return position == nullptr ? DefaultProperties::Position : *position;
			}

			static vec4 SourceOrDefault(const vec4* source, const D3D_Texture2D* texture)
			{
				return source == nullptr ? vec4(0.0f, 0.0f, texture->GetSize()) : *source;
			}

			static vec2 SizeOrDefault(const D3D_Texture2D* texture, const vec4* source)
			{
				return texture != nullptr ? vec2(texture->GetSize()) : vec2(source->w, source->z);
			}

			static vec2 OriginOrDefault(const vec2* origin)
			{
				return origin == nullptr ? DefaultProperties::Origin : -*origin;
			}

			static vec2 ScaleOrDefault(const vec2* scale)
			{
				return scale == nullptr ? DefaultProperties::Scale : *scale;
			}

			static vec4 ColorOrDefault(const vec4* color)
			{
				return color == nullptr ? DefaultProperties::Color : *color;
			}
		};

		enum SpriteShaderTextureSlot
		{
			TextureSpriteSlot = 0,
			TextureMaskSlot = 1
		};
	}

	D3D_Renderer2D::D3D_Renderer2D()
		: spriteShader(Sprite_VS(), Sprite_PS(), "Renderer2D::Sprite")
	{
		D3D_SetObjectDebugName(cameraConstantBuffer.Buffer.GetBuffer(), "Renderer2D::CameraConstantBuffer");
		D3D_SetObjectDebugName(spriteConstantBuffer.Buffer.GetBuffer(), "Renderer2D::SpriteConstantBuffer");

		D3D_SetObjectDebugName(rasterizerState.GetRasterizerState(), "Renderer2D::RasterizerState");

		InternalCreateIndexBuffer();
		InternalCreateVertexBuffer();
		InternalCreateInputLayout();
	}

	void D3D_Renderer2D::InternalCreateIndexBuffer()
	{
		std::array<SpriteIndices, MaxBatchItemSize> indexData;
		for (uint16_t i = 0, offset = 0; i < indexData.size(); i++)
		{
			// [0] TopLeft	  - [1] TopRight
			// [2] BottomLeft - [3] BottomRight;
			enum { TopLeft = 0, TopRight = 1, BottomLeft = 2, BottomRight = 3 };

			indexData[i] =
			{
				// NOTE: Used to be counter clockwise for OpenGL but D3D's winding order is clockwise by default
				static_cast<uint16_t>(offset + TopLeft),
				static_cast<uint16_t>(offset + TopRight),
				static_cast<uint16_t>(offset + BottomRight),

				static_cast<uint16_t>(offset + BottomRight),
				static_cast<uint16_t>(offset + BottomLeft),
				static_cast<uint16_t>(offset + TopLeft),
			};

			offset += static_cast<uint16_t>(SpriteVertices::GetVertexCount());
		}

		indexBuffer = MakeUnique<D3D_StaticIndexBuffer>(indexData.size(), indexData.data(), IndexType::UInt16);
		D3D_SetObjectDebugName(indexBuffer->GetBuffer(), "Renderer2D::IndexBuffer");
	}

	void D3D_Renderer2D::InternalCreateVertexBuffer()
	{
		vertexBuffer = MakeUnique<D3D_DynamicVertexBuffer>(MaxBatchItemSize * sizeof(SpriteVertex), nullptr, sizeof(SpriteVertex));
		D3D_SetObjectDebugName(vertexBuffer->GetBuffer(), "Renderer2D::VertexBuffer");
	}

	void D3D_Renderer2D::InternalCreateInputLayout()
	{
		static constexpr InputElement elements[] =
		{
			{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(SpriteVertex, Position)				},
			{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(SpriteVertex, TextureCoordinates)		},
			{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	offsetof(SpriteVertex, TextureMaskCoordinates)	},
			{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	offsetof(SpriteVertex, Color)					},
		};

		inputLayout = MakeUnique<D3D_InputLayout>(elements, std::size(elements), spriteShader.VS);
		D3D_SetObjectDebugName(inputLayout->GetLayout(), "Renderer2D::InputLayout");
	}

	void D3D_Renderer2D::InternalCreateBatches()
	{
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

	void D3D_Renderer2D::InternalFlush()
	{
		rasterizerState.Bind();
		D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		std::array<ID3D11SamplerState*, 2> samplerStates = 
		{ 
			// NOTE: Sprite sampler
			defaultTextureSampler.GetSampler(), 
			// NOTE: Sprite mask sampler
			defaultTextureSampler.GetSampler(),
		};
		D3D.Context->PSSetSamplers(0, static_cast<UINT>(samplerStates.size()), samplerStates.data());

		spriteShader.Bind();

		vertexBuffer->Bind();
		vertexBuffer->UploadData(vertices.size() * sizeof(SpriteVertices), vertices.data());

		inputLayout->Bind();
		indexBuffer->Bind();

		cameraConstantBuffer.Data.ViewProjection = glm::transpose(orthographicCamera->GetProjectionMatrix() * orthographicCamera->GetViewMatrix());
		cameraConstantBuffer.UploadData();
		cameraConstantBuffer.BindVertexShader();

		spriteConstantBuffer.Data.DrawTextBorder = drawTextBorder;
		spriteConstantBuffer.BindPixelShader();

		InternalCreateBatches();
		AetBlendMode lastBlendMode = AetBlendMode::Normal;

		for (uint16_t i = 0; i < batches.size(); i++)
		{
			const SpriteBatch& batch = batches[i];
			const SpriteBatchItem& item = batchItems[batch.Index];

			const bool firstItem = i == 0;
			if (firstItem || lastBlendMode != item.BlendMode)
			{
				InternalSetBlendMode(item.BlendMode);
				lastBlendMode = item.BlendMode;
			}

			if (item.Texture != nullptr)
				item.Texture->Bind(TextureSpriteSlot);

			if (item.MaskTexture != nullptr)
				item.MaskTexture->Bind(TextureMaskSlot);

			spriteConstantBuffer.Data.Format = (item.Texture == nullptr) ? TextureFormat::Unknown : item.Texture->GetTextureFormat();
			spriteConstantBuffer.Data.MaskFormat = (item.MaskTexture == nullptr) ? TextureFormat::Unknown : item.MaskTexture->GetTextureFormat();
			spriteConstantBuffer.Data.BlendMode = item.BlendMode;
			spriteConstantBuffer.Data.DrawCheckerboard = item.CheckerboardSize != vec2(0.0f, 0.0f);
			spriteConstantBuffer.Data.CheckerboardSize = item.CheckerboardSize;

			spriteConstantBuffer.UploadData();

			D3D.Context->DrawIndexed(
				batch.Count * SpriteIndices::GetIndexCount(),
				batch.Index * SpriteIndices::GetIndexCount(),
				0);

			drawCallCount++;
		}

		rasterizerState.UnBind();
		InternalClearItems();
	}

	void D3D_Renderer2D::InternalCheckFlushItems()
	{
		// TODO: Something isn't quite right here...
		if (batchItems.size() >= MaxBatchItemSize / 12)
			InternalFlush();
	}

	void D3D_Renderer2D::InternalSetBlendMode(AetBlendMode blendMode)
	{
		switch (blendMode)
		{
		default:
		case AetBlendMode::Normal:
			aetBlendStates.Normal.Bind();
			break;
		case AetBlendMode::Add:
			aetBlendStates.Add.Bind();
			break;
		case AetBlendMode::Multiply:
			aetBlendStates.Multiply.Bind();
			break;
		case AetBlendMode::LinearDodge:
			aetBlendStates.LinearDodge.Bind();
			break;
		case AetBlendMode::Overlay:
			aetBlendStates.Overlay.Bind();
			break;
		}
	}

	SpriteBatchPair D3D_Renderer2D::InternalCheckFlushAddItem()
	{
		InternalCheckFlushItems();
		return InternalAddItem();
	}

	SpriteBatchPair D3D_Renderer2D::InternalAddItem()
	{
		batchItems.emplace_back();
		vertices.emplace_back();

		return { &batchItems.back(), &vertices.back() };
	}

	void D3D_Renderer2D::InternalClearItems()
	{
		batches.clear();
		batchItems.clear();
		vertices.clear();
	}

	void D3D_Renderer2D::InternalDraw(const D3D_Texture2D * texture, const vec4 * sourceRegion, const vec2 * position, const vec2 * origin, float rotation, const vec2 * scale, const vec4 * color, AetBlendMode blendMode)
	{
		SpriteBatchPair pair = InternalCheckFlushAddItem();

		pair.Item->SetValues(
			texture,
			nullptr,
			blendMode);

		pair.Vertices->SetValues(
			DefaultProperties::PositionOrDefault(position),
			DefaultProperties::SourceOrDefault(sourceRegion, texture),
			DefaultProperties::SizeOrDefault(texture, sourceRegion),
			DefaultProperties::OriginOrDefault(origin),
			rotation,
			DefaultProperties::ScaleOrDefault(scale),
			DefaultProperties::ColorOrDefault(color));
	}

	void D3D_Renderer2D::Begin(const OrthographicCamera& camera)
	{
		drawCallCount = 0;
		orthographicCamera = &camera;
	}

	void D3D_Renderer2D::Draw(vec2 position, vec2 size, vec4 color)
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		InternalDraw(nullptr, &source, &position, nullptr, DefaultProperties::Rotation, nullptr, &color);
	}

	void D3D_Renderer2D::Draw(vec2 position, vec2 size, const vec4 colors[4])
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);

		SpriteBatchPair pair = InternalCheckFlushAddItem();
		pair.Item->SetValues(nullptr, nullptr);
		pair.Vertices->SetValues(position, source, size, DefaultProperties::Origin, DefaultProperties::Rotation, DefaultProperties::Scale, colors);
	}

	void D3D_Renderer2D::Draw(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color)
	{
		const vec4 source = vec4(0.0f, 0.0f, size.x, size.y);
		InternalDraw(nullptr, &source, &position, &origin, rotation, &scale, &color);
	}

	void D3D_Renderer2D::Draw(const D3D_Texture2D* texture, vec2 position, vec4 color)
	{
		InternalDraw(texture, nullptr, &position, nullptr, DefaultProperties::Rotation, nullptr, &color);
	}

	void D3D_Renderer2D::Draw(const D3D_Texture2D* texture, vec4 sourceRegion, vec2 position, vec4 color)
	{
		InternalDraw(texture, &sourceRegion, &position, nullptr, 0.0f, nullptr, &color);
	}

	void D3D_Renderer2D::Draw(const D3D_Texture2D* texture, vec2 position, vec2 origin, float rotation, vec4 color)
	{
		InternalDraw(texture, nullptr, &position, &origin, rotation, nullptr, &color);
	}

	void D3D_Renderer2D::Draw(const D3D_Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color, AetBlendMode blendMode)
	{
		InternalDraw(texture, &sourceRegion, &position, &origin, rotation, &scale, &color, blendMode);
	}

	void D3D_Renderer2D::Draw(
		const D3D_Texture2D* maskTexture, vec4 maskSourceRegion, vec2 maskPosition, vec2 maskOrigin, float maskRotation, vec2 maskScale,
		const D3D_Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color,
		AetBlendMode blendMode)
	{
		SpriteBatchPair pair = InternalCheckFlushAddItem();

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

	void D3D_Renderer2D::DrawLine(vec2 start, vec2 end, vec4 color, float thickness)
	{
		const vec2 edge = end - start;

		const float distance = glm::distance(start, end);
		const float angle = glm::degrees(glm::atan(edge.y, edge.x));

		const vec4 source = vec4(0.0f, 0.0f, distance, thickness);
		const vec2 origin = vec2(0.0f, thickness / 2.0f);

		InternalDraw(nullptr, &source, &start, &origin, angle, nullptr, &color);
	}

	void D3D_Renderer2D::DrawLine(vec2 start, float angle, float length, vec4 color, float thickness)
	{
		const vec4 source = vec4(0.0f, 0.0f, length, thickness);
		const vec2 origin = vec2(0.0f, thickness / 2.0f);

		InternalDraw(nullptr, &source, &start, &origin, angle, nullptr, &color);
	}

	void D3D_Renderer2D::DrawRectangle(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, vec4 color, float thickness)
	{
		DrawLine(topLeft, topRight, color, thickness);
		DrawLine(topRight, bottomRight, color, thickness);
		DrawLine(bottomRight, bottomLeft, color, thickness);
		DrawLine(bottomLeft, topLeft, color, thickness);
	}

	void D3D_Renderer2D::DrawCheckerboardRectangle(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color, float precision)
	{
		// TODO: Use checkerboardTexture instead
		const vec4 source = vec4(0.0f, 0.0f, size);
		InternalDraw(nullptr, &source, &position, &origin, rotation, &scale, &color);

		batchItems.back().CheckerboardSize = size * scale * precision;
	}

	void D3D_Renderer2D::End()
	{
		InternalFlush();
		orthographicCamera = nullptr;
	}

	const SpriteVertices& D3D_Renderer2D::GetLastVertices() const
	{
		return vertices.back();
	}

	const OrthographicCamera* D3D_Renderer2D::GetCamera() const
	{
		return orthographicCamera;
	}

	bool D3D_Renderer2D::GetDrawTextBorder() const
	{
		return drawTextBorder;
	}

	void D3D_Renderer2D::SetDrawTextBorder(bool value)
	{
		drawTextBorder = value;
	}

	uint32_t D3D_Renderer2D::GetDrawCallCount() const
	{
		return drawCallCount;
	}
}
