#include "Renderer2D.h"
#include "Detail/RenderTarget2DImpl.h"
#include "Detail/SpriteBatchData.h"
#include "Render/D3D11/GraphicsResourceUtil.h"
#include "Render/D3D11/Buffer/ConstantBuffer.h"
#include "Render/D3D11/Buffer/IndexBuffer.h"
#include "Render/D3D11/Buffer/VertexBuffer.h"
#include "Render/D3D11/Shader/Bytecode/ShaderBytecode.h"
#include "Render/D3D11/State/BlendState.h"
#include "Render/D3D11/State/DepthStencilState.h"
#include "Render/D3D11/State/InputLayout.h"
#include "Render/D3D11/State/RasterizerState.h"
#include "Render/D3D11/Texture/RenderTarget.h"
#include "Render/D3D11/Texture/TextureSampler.h"

// TODO: Implement using bound textures array + per vertex sprite / mask indices to allow for multi texture sprite batching
// #define COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH

namespace Comfy::Render
{
	using namespace Graphics;

	vec2 TextureOrRegionSize(const D3D11::Texture2D* texture, const vec4& source)
	{
		return (texture != nullptr) ? vec2(texture->GetSize()) : vec2(source.z, source.w);
	}

#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
	enum SpriteShaderTextureSlot
	{
		TextureSpriteSlot = 0,
		TextureMaskSlot = 1
	};
#endif

	struct Renderer2D::Impl
	{
	public:
		// TODO: Move to Detail namespace and directory
		struct CameraConstantData
		{
			mat4 ViewProjection;
		};

		struct ArrayPaddedTextureFormat { TextureFormat Value; i32 Padding[3]; };

		struct SpriteConstantData
		{
#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
			TextureFormat Format;
			TextureFormat MaskFormat;
#else
			//u32 ArrayPadding[3];
#endif
			AetBlendMode BlendMode;
			u8 BlendModePadding[3];
			int Flags;
			int DrawTextBorder;
			int DrawCheckerboard;

			vec4 CheckerboardSize;

#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
			//u32 TrailingPadding[4];

			ArrayPaddedTextureFormat TextureMaskFormat;
			std::array<ArrayPaddedTextureFormat, SpriteTextureSlots> TextureFormats;
#else
			u32 PADDING[2];
#endif
		};

		static inline const auto test = offsetof(SpriteConstantData, BlendMode);

	public:
		AetRenderer AetRenderer;

		u32 DrawCallCount = 0;

		D3D11::ShaderPair SpriteShader = { D3D11::Sprite_VS(), D3D11::Sprite_PS(), "Renderer2D::Sprite" };

		D3D11::DefaultConstantBufferTemplate<CameraConstantData> CameraConstantBuffer = { 0 };
		D3D11::DynamicConstantBufferTemplate<SpriteConstantData> SpriteConstantBuffer = { 0 };

		std::unique_ptr<D3D11::StaticIndexBuffer> IndexBuffer = nullptr;
		std::unique_ptr<D3D11::DynamicVertexBuffer> VertexBuffer = nullptr;
		std::unique_ptr<D3D11::InputLayout> InputLayout = nullptr;

		// NOTE: Disable backface culling for negatively scaled sprites
		D3D11::RasterizerState RasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE };

		// TODO: Once needed the Renderer2D should expose optional wrapped address modes
		D3D11::TextureSampler DefaultTextureSampler = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER };

		/*
		// TODO: Implement using CheckerboardTexture instead (?)
		static constexpr std::array<u32, 4> CheckerboardTexturePixels =
		{
			0xFFFFFFFF, 0x00000000,
			0x00000000, 0xFFFFFFFF,
		};

		ImmutableTexture2D CheckerboardTexture = { ivec2(2, 2), checkerboardTexturePixels.data(), D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP };
		*/

		struct AetBlendStates
		{
			D3D11::BlendState Normal = { AetBlendMode::Normal };
			D3D11::BlendState Add = { AetBlendMode::Add };
			D3D11::BlendState Multiply = { AetBlendMode::Multiply };
			D3D11::BlendState LinearDodge = { AetBlendMode::LinearDodge };
			D3D11::BlendState Overlay = { AetBlendMode::Overlay };
		} AetBlendStates;

		std::vector<Detail::SpriteBatch> Batches;
		std::vector<Detail::SpriteBatchItem> BatchItems;
		std::vector<Detail::SpriteVertices> Vertices;

		OrthographicCamera* OrthographicCamera = nullptr;
		Detail::RenderTarget2DImpl* RenderTarget = nullptr;

	public:
		Impl(Renderer2D& parent) : AetRenderer(parent)
		{
			D3D11_SetObjectDebugName(CameraConstantBuffer.Buffer.GetBuffer(), "Renderer2D::CameraConstantBuffer");
			D3D11_SetObjectDebugName(SpriteConstantBuffer.Buffer.GetBuffer(), "Renderer2D::SpriteConstantBuffer");

			D3D11_SetObjectDebugName(RasterizerState.GetRasterizerState(), "Renderer2D::RasterizerState");

			InternalCreateIndexBuffer();
			InternalCreateVertexBuffer();
			InternalCreateInputLayout();
		}

		void InternalCreateIndexBuffer()
		{
			std::array<Detail::SpriteIndices, Renderer2D::MaxBatchItemSize> indexData;
			for (u16 i = 0, offset = 0; i < indexData.size(); i++)
			{
				// [0] TopLeft	  - [1] TopRight
				// [2] BottomLeft - [3] BottomRight;
				enum { TopLeft = 0, TopRight = 1, BottomLeft = 2, BottomRight = 3 };

				indexData[i] =
				{
					// NOTE: Used to be counter clockwise for OpenGL but D3D's winding order is clockwise by default
					static_cast<u16>(offset + TopLeft),
					static_cast<u16>(offset + TopRight),
					static_cast<u16>(offset + BottomRight),

					static_cast<u16>(offset + BottomRight),
					static_cast<u16>(offset + BottomLeft),
					static_cast<u16>(offset + TopLeft),
				};

				offset += static_cast<u16>(Detail::SpriteVertices::GetVertexCount());
			}

			IndexBuffer = std::make_unique<D3D11::StaticIndexBuffer>(indexData.size(), indexData.data(), IndexFormat::U16);
			D3D11_SetObjectDebugName(IndexBuffer->GetBuffer(), "Renderer2D::IndexBuffer");
		}

		void InternalCreateVertexBuffer()
		{
			VertexBuffer = std::make_unique<D3D11::DynamicVertexBuffer>(Renderer2D::MaxBatchItemSize * sizeof(Detail::SpriteVertex), nullptr, sizeof(Detail::SpriteVertex));
			D3D11_SetObjectDebugName(VertexBuffer->GetBuffer(), "Renderer2D::VertexBuffer");
		}

		void InternalCreateInputLayout()
		{
			static constexpr D3D11::InputElement elements[] =
			{
				{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(Detail::SpriteVertex, Position)				},
				{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(Detail::SpriteVertex, TextureCoordinates)		},
				{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	offsetof(Detail::SpriteVertex, TextureMaskCoordinates)	},
				{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	offsetof(Detail::SpriteVertex, Color)					},
#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
				{ "TEXINDEX",	0, DXGI_FORMAT_R32_UINT,		offsetof(Detail::SpriteVertex, TextureIndex)			},
#endif
			};

			InputLayout = std::make_unique<D3D11::InputLayout>(elements, std::size(elements), SpriteShader.VS);
			D3D11_SetObjectDebugName(InputLayout->GetLayout(), "Renderer2D::InputLayout");
		}

#if !defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
		int FindAvailableTextureArrayIndex(Detail::SpriteBatch& currentBatch, const D3D11::Texture2D* texture) const
		{
			for (int i = 0; i < static_cast<int>(SpriteTextureSlots); i++)
			{
				if (currentBatch.Textures[i] == nullptr || currentBatch.Textures[i] == texture)
					return i;
			}

			return -1;
		}
#endif

		void InternalCreateBatches()
		{
#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
			for (u16 i = 0; i < BatchItems.size(); i++)
			{
				const bool isFirst = (i == 0);
				const Detail::SpriteBatchItem* item = &BatchItems[i];
				const Detail::SpriteBatchItem* lastItem = isFirst ? nullptr : &BatchItems[Batches.back().Index];

				constexpr vec2 zeroSize = vec2(0.0f);
				const bool newBatch = isFirst ||
					(item->BlendMode != lastItem->BlendMode) ||
					(item->DrawTextBorder != lastItem->DrawTextBorder) ||
					(item->Texture != lastItem->Texture) ||
					(item->MaskTexture != nullptr) ||
					(item->CheckerboardSize != zeroSize || lastItem->CheckerboardSize != zeroSize);

				if (newBatch)
					Batches.emplace_back(i, 1);
				else
					Batches.back().Count++;
			}
#else
			if (BatchItems.empty())
				return;

			Batches.emplace_back(0, 1).Textures[0] = BatchItems.front().Texture;
			Vertices.front().SetTextureIndices((BatchItems.front().Texture == nullptr) ? -1 : 0);

			for (u16 i = 1; i < BatchItems.size(); i++)
			{
				const Detail::SpriteBatchItem& item = BatchItems[i];
				const Detail::SpriteBatchItem& lastItem = BatchItems[Batches.back().Index];

				constexpr vec2 zeroSize = vec2(0.0f);
				const bool requiresNewBatch =
					(item.BlendMode != lastItem.BlendMode) ||
					(item.DrawTextBorder != lastItem.DrawTextBorder) ||
					// (item.Texture != lastItem.Texture) ||
					(item.MaskTexture != lastItem.MaskTexture) ||
					(item.CheckerboardSize != zeroSize || lastItem.CheckerboardSize != zeroSize);

				if (item.Texture == nullptr)
				{
					if (requiresNewBatch)
						Batches.emplace_back(i, 1);
					else
						Batches.back().Count++;

					Vertices[i].SetTextureIndices(-1);
				}
				else if (const int availableTextureIndex = FindAvailableTextureArrayIndex(Batches.back(), item.Texture); availableTextureIndex >= 0)
				{
					if (requiresNewBatch)
						Batches.emplace_back(i, 1);
					else
						Batches.back().Count++;

					Vertices[i].SetTextureIndices(availableTextureIndex);
					Batches.back().Textures[availableTextureIndex] = item.Texture;
				}
				else
				{
					Batches.emplace_back(i, 1);

					Vertices[i].SetTextureIndices(0);
					Batches.back().Textures[0] = item.Texture;
				}
			}
#endif
		}

		void InternalFlush(bool finalFlush)
		{
			InternalCreateBatches();

			RenderTarget->Main.ResizeIfDifferent(RenderTarget->Param.Resolution);
			RenderTarget->Main.SetMultiSampleCountIfDifferent(RenderTarget->Param.MultiSampleCount);
			RenderTarget->Main.BindSetViewport();

			if (const bool isFirstFlush = (DrawCallCount == 0); isFirstFlush)
				RenderTarget->Main.Clear(RenderTarget->Param.ClearColor);

			RasterizerState.Bind();
			D3D11::D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			SpriteShader.Bind();

			VertexBuffer->Bind();
			VertexBuffer->UploadData(Vertices.size() * sizeof(Detail::SpriteVertices), Vertices.data());

			InputLayout->Bind();
			IndexBuffer->Bind();

			OrthographicCamera->UpdateMatrices();
			CameraConstantBuffer.Data.ViewProjection = glm::transpose(OrthographicCamera->GetViewProjection());
			CameraConstantBuffer.UploadData();
			CameraConstantBuffer.BindVertexShader();

			SpriteConstantBuffer.BindPixelShader();

			AetBlendMode lastBlendMode = AetBlendMode::Count;

#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
			D3D11::ShaderResourceView::BindArray<2>(0, { nullptr, nullptr });
			D3D11::TextureSampler::BindArray<2>(0, { &DefaultTextureSampler, &DefaultTextureSampler });
#else
			DefaultTextureSampler.Bind(0);
#endif

			for (u16 i = 0; i < Batches.size(); i++)
			{
				const Detail::SpriteBatch& batch = Batches[i];
				const Detail::SpriteBatchItem& item = BatchItems[batch.Index];

				if (lastBlendMode != item.BlendMode)
				{
					InternalSetBlendMode(item.BlendMode);
					lastBlendMode = item.BlendMode;
				}

#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
				if (item.Texture != nullptr)
					item.Texture->Bind(TextureSpriteSlot);

				if (item.MaskTexture != nullptr)
					item.MaskTexture->Bind(TextureMaskSlot);
#else
				std::array<const D3D11::ShaderResourceView*, SpriteTextureSlots + 1> textureResourceViews;
				textureResourceViews.front() = item.MaskTexture;
				for (size_t i = 0; i < SpriteTextureSlots; i++)
					textureResourceViews[i + 1] = batch.Textures[i];

				D3D11::ShaderResourceView::BindArray(0, textureResourceViews);
#endif

#if defined(COMFY_RENDERER2D_SINGLE_TEXTURE_BATCH)
				SpriteConstantBuffer.Data.Format = (item.Texture == nullptr) ? TextureFormat::Unknown : item.Texture->GetTextureFormat();
				SpriteConstantBuffer.Data.MaskFormat = (item.MaskTexture == nullptr) ? TextureFormat::Unknown : item.MaskTexture->GetTextureFormat();
#else
				SpriteConstantBuffer.Data.TextureMaskFormat.Value = (item.MaskTexture == nullptr) ? TextureFormat::Unknown : item.MaskTexture->GetTextureFormat();
				for (size_t i = 0; i < SpriteTextureSlots; i++)
					SpriteConstantBuffer.Data.TextureFormats[i].Value = (batch.Textures[i] == nullptr) ? TextureFormat::Unknown : batch.Textures[i]->GetTextureFormat();
#endif

				SpriteConstantBuffer.Data.BlendMode = item.BlendMode;
				SpriteConstantBuffer.Data.DrawTextBorder = item.DrawTextBorder;
				SpriteConstantBuffer.Data.DrawCheckerboard = item.CheckerboardSize != vec2(0.0f, 0.0f);
				SpriteConstantBuffer.Data.CheckerboardSize = vec4(item.CheckerboardSize, 0.0f, 0.0f);

				SpriteConstantBuffer.UploadData();

				D3D11::D3D.Context->DrawIndexed(
					batch.Count * Detail::SpriteIndices::GetIndexCount(),
					batch.Index * Detail::SpriteIndices::GetIndexCount(),
					0);

				DrawCallCount++;
			}

			RasterizerState.UnBind();
			InternalClearItems();

			RenderTarget->Main.UnBind();

			if (finalFlush && RenderTarget->Param.MultiSampleCount > 1)
			{
				RenderTarget->ResolvedMain.ResizeIfDifferent(RenderTarget->Param.Resolution);
				D3D11::D3D.Context->ResolveSubresource(RenderTarget->ResolvedMain.GetResource(), 0, RenderTarget->Main.GetResource(), 0, RenderTarget->Main.GetBackBufferDescription().Format);
			}
		}

		void InternalSetBlendMode(AetBlendMode blendMode)
		{
			switch (blendMode)
			{
			default:
			case AetBlendMode::Normal:
				AetBlendStates.Normal.Bind();
				break;
			case AetBlendMode::Add:
				AetBlendStates.Add.Bind();
				break;
			case AetBlendMode::Multiply:
				AetBlendStates.Multiply.Bind();
				break;
			case AetBlendMode::LinearDodge:
				AetBlendStates.LinearDodge.Bind();
				break;
			case AetBlendMode::Overlay:
				AetBlendStates.Overlay.Bind();
				break;
			}
		}

		Detail::SpriteBatchPair InternalCheckFlushAddItem()
		{
			// TODO: Something isn't quite right here...
			if (BatchItems.size() >= Renderer2D::MaxBatchItemSize / 12)
				InternalFlush(false);

			return InternalAddItem();
		}

		Detail::SpriteBatchPair InternalAddItem()
		{
			BatchItems.emplace_back();
			Vertices.emplace_back();

			return { &BatchItems.back(), &Vertices.back() };
		}

		void InternalClearItems()
		{
			Batches.clear();
			BatchItems.clear();
			Vertices.clear();
		}

		void InternalDraw(const RenderCommand2D& command)
		{
			Detail::SpriteBatchPair pair = InternalCheckFlushAddItem();

			pair.Item->Texture = D3D11::GetTexture2D(command.Texture);
			pair.Item->MaskTexture = nullptr;
			pair.Item->BlendMode = command.BlendMode;
			pair.Item->DrawTextBorder = command.DrawTextBorder;

			pair.Vertices->SetValues(
				command.Position,
				command.SourceRegion,
				TextureOrRegionSize(D3D11::GetTexture2D(command.Texture), command.SourceRegion),
				-command.Origin,
				command.Rotation,
				command.Scale,
				command.CornerColors.data());
		}

		void InternalDraw(const RenderCommand2D& command, const RenderCommand2D& commandMask)
		{
			Detail::SpriteBatchPair pair = InternalCheckFlushAddItem();

			pair.Item->Texture = D3D11::GetTexture2D(command.Texture);
			pair.Item->MaskTexture = D3D11::GetTexture2D(commandMask.Texture);
			pair.Item->BlendMode = command.BlendMode;
			pair.Item->DrawTextBorder = command.DrawTextBorder;

			pair.Vertices->SetValues(
				commandMask.Position,
				commandMask.SourceRegion,
				TextureOrRegionSize(D3D11::GetTexture2D(commandMask.Texture), commandMask.SourceRegion),
				-commandMask.Origin,
				commandMask.Rotation,
				commandMask.Scale,
				commandMask.CornerColors.data());

			pair.Vertices->SetTexMaskCoords(
				D3D11::GetTexture2D(command.Texture),
				command.Position,
				command.Scale,
				command.Origin + vec2(command.SourceRegion.x, command.SourceRegion.y),
				command.Rotation,
				commandMask.Position,
				commandMask.Scale,
				commandMask.Origin,
				commandMask.Rotation,
				commandMask.SourceRegion);
		}
	};

	Renderer2D::Renderer2D() : impl(std::make_unique<Impl>(*this))
	{
	}

	Renderer2D::~Renderer2D()
	{
	}

	void Renderer2D::Begin(OrthographicCamera& camera, RenderTarget2D& renderTarget)
	{
		assert(impl->OrthographicCamera == nullptr);

		impl->DrawCallCount = 0;
		impl->OrthographicCamera = &camera;
		impl->RenderTarget = static_cast<Detail::RenderTarget2DImpl*>(&renderTarget);
	}

	void Renderer2D::Draw(const RenderCommand2D& command)
	{
		impl->InternalDraw(command);
	}

	void Renderer2D::Draw(const RenderCommand2D& command, const RenderCommand2D& commandMask)
	{
		impl->InternalDraw(command, commandMask);
	}

	void Renderer2D::DrawLine(vec2 start, vec2 end, const vec4& color, float thickness)
	{
		const auto edge = (end - start);

		RenderCommand2D command;
		command.SourceRegion = vec4(0.0f, 0.0f, glm::distance(start, end), thickness);
		command.Position = start;
		command.Origin = vec2(0.0f, thickness / 2.0f);
		command.Rotation = glm::degrees(glm::atan(edge.y, edge.x));
		command.CornerColors = { color, color, color, color };
		impl->InternalDraw(command);
	}

	void Renderer2D::DrawLine(vec2 start, float angle, float length, const vec4& color, float thickness)
	{
		RenderCommand2D command;
		command.SourceRegion = vec4(0.0f, 0.0f, length, thickness);
		command.Position = start;
		command.Origin = vec2(0.0f, thickness / 2.0f);
		command.Rotation = angle;
		command.CornerColors = { color, color, color, color };
		impl->InternalDraw(command);
	}

	void Renderer2D::DrawRect(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, const vec4& color, float thickness)
	{
		DrawLine(topLeft, topRight, color, thickness);
		DrawLine(topRight, bottomRight, color, thickness);
		DrawLine(bottomRight, bottomLeft, color, thickness);
		DrawLine(bottomLeft, topLeft, color, thickness);
	}

	void Renderer2D::DrawRectCheckerboard(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, const vec4& color, float precision)
	{
		RenderCommand2D command;
		command.SourceRegion = vec4(0.0f, 0.0f, size);
		command.Position = position;
		command.Origin = origin;
		command.Rotation = rotation;
		command.Scale = scale;
		command.CornerColors = { color, color, color, color };
		impl->InternalDraw(command);
		impl->BatchItems.back().CheckerboardSize = (size * scale * precision);
	}

	AetRenderer& Renderer2D::Aet()
	{
		return impl->AetRenderer;
	}

	const OrthographicCamera& Renderer2D::GetCamera() const
	{
		assert(impl->OrthographicCamera != nullptr);
		return *impl->OrthographicCamera;
	}

	RenderTarget2D& Renderer2D::GetRenderTarget() const
	{
		assert(impl->RenderTarget != nullptr);
		return *impl->RenderTarget;
	}

	std::unique_ptr<RenderTarget2D> Renderer2D::CreateRenderTarget()
	{
		return std::make_unique<Detail::RenderTarget2DImpl>();
	}

	void Renderer2D::End()
	{
		assert(impl->OrthographicCamera != nullptr);

		impl->InternalFlush(true);
		impl->OrthographicCamera = nullptr;
		impl->RenderTarget = nullptr;
	}
}
