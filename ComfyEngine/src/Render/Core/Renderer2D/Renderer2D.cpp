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

namespace Comfy::Render
{
	using namespace Graphics;

	vec2 TextureOrRegionSize(const D3D11::Texture2D* texture, const vec4& source)
	{
		return (texture != nullptr) ? vec2(texture->GetSize()) : vec2(source.z, source.w);
	}

	struct Renderer2D::Impl
	{
	public:
		static constexpr u32 MaxBatchItemSize = 2048;

	public:
		struct CameraConstantData
		{
			mat4 ViewProjection;
		};

		struct SpriteConstantData
		{
			AetBlendMode BlendMode;
			u8 BlendModePadding[3];

			TextureFormat Format;
			TextureFormat MaskFormat;
			u32 FormatFlags;

			vec4 CheckerboardSize;
		};

		static_assert(MaxSpriteTextureSlots <= (sizeof(SpriteConstantData::FormatFlags) * CHAR_BIT));

	public:
		AetRenderer AetRenderer;

		u32 DrawCallCount = 0;

		struct RendererShaderPairs
		{
			struct MultiTextureShaders
			{
				std::array<D3D11::ShaderPair, MaxSpriteTextureSlots> TextureBatch =
				{
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_01_PS(), "Renderer2D::SpriteMultiTextureBatch_01_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_02_PS(), "Renderer2D::SpriteMultiTextureBatch_02_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_03_PS(), "Renderer2D::SpriteMultiTextureBatch_03_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_04_PS(), "Renderer2D::SpriteMultiTextureBatch_04_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_05_PS(), "Renderer2D::SpriteMultiTextureBatch_05_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_06_PS(), "Renderer2D::SpriteMultiTextureBatch_06_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_07_PS(), "Renderer2D::SpriteMultiTextureBatch_07_PS" },
					D3D11::ShaderPair { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatch_08_PS(), "Renderer2D::SpriteMultiTextureBatch_08_PS" },
				};
				D3D11::ShaderPair TextureBatchMultiply = { D3D11::SpriteMultiTexture_VS(), D3D11::SpriteMultiTextureBatchBlend_08_PS(), "Renderer2D::SpriteMultiTextureBatchBlend_08" };
			} Multi;

			struct SingleTextureShaders
			{
				D3D11::ShaderPair TextureCheckerboard = { D3D11::SpriteSingleTexture_VS(), D3D11::SpriteSingleTextureCheckerboard_PS(), "Renderer2D::SpriteSingleTextureCheckerboard" };
				D3D11::ShaderPair TextureFont = { D3D11::SpriteSingleTexture_VS(), D3D11::SpriteSingleTextureFont_PS(), "Renderer2D::SpriteSingleTextureFont" };
				D3D11::ShaderPair TextureMask = { D3D11::SpriteSingleTexture_VS(), D3D11::SpriteSingleTextureMask_PS(), "Renderer2D::SpriteSingleTextureMask" };
				D3D11::ShaderPair TextureMaskMultiply = { D3D11::SpriteSingleTexture_VS(), D3D11::SpriteSingleTextureMaskBlend_PS(), "Renderer2D::SpriteSingleTextureMaskBlend" };
			} Single;
		} Shaders;

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

		// NOTE: Avoid additional branches by using a 1x1 white fallback texture for rendering solid color
		static constexpr std::array<u32, 1> WhiteTexturePixels = { 0xFFFFFFFF };
		D3D11::Texture2D WhiteTexture = { ivec2(1, 1), WhiteTexturePixels.data() };

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
			D3D11_SetObjectDebugName(WhiteTexture.GetTexture(), "Renderer2D::WhiteTexture");

			InternalCreateIndexBuffer();
			InternalCreateVertexBuffer();
			InternalCreateInputLayout();
		}

		void InternalCreateIndexBuffer()
		{
			std::array<Detail::SpriteIndices, MaxBatchItemSize> indexData;

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

			IndexBuffer = std::make_unique<D3D11::StaticIndexBuffer>(indexData.size() * sizeof(Detail::SpriteIndices), indexData.data(), IndexFormat::U16);
			D3D11_SetObjectDebugName(IndexBuffer->GetBuffer(), "Renderer2D::IndexBuffer");
		}

		void InternalCreateVertexBuffer()
		{
			VertexBuffer = std::make_unique<D3D11::DynamicVertexBuffer>(MaxBatchItemSize * sizeof(Detail::SpriteVertices), nullptr, sizeof(Detail::SpriteVertex));
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
				{ "TEXINDEX",	0, DXGI_FORMAT_R32_UINT,		offsetof(Detail::SpriteVertex, TextureIndex)			},
			};

			InputLayout = std::make_unique<D3D11::InputLayout>(elements, std::size(elements), Shaders.Multi.TextureBatch[0].VS);
			D3D11_SetObjectDebugName(InputLayout->GetLayout(), "Renderer2D::InputLayout");
		}

		int FindAvailableTextureSlot(Detail::SpriteBatch& currentBatch, const D3D11::Texture2D* texture) const
		{
			for (int i = 0; i < static_cast<int>(MaxSpriteTextureSlots); i++)
			{
				if (currentBatch.Textures[i] == nullptr || currentBatch.Textures[i] == texture)
					return i;
			}

			return -1;
		}

		bool AreBatchItemsCompatible(const Detail::SpriteBatchItem& item, const Detail::SpriteBatchItem& lastItem) const
		{
			const bool textureChanged = (item.Texture != lastItem.Texture);
			const bool textureMaskChanged = (item.MaskTexture != lastItem.MaskTexture);

			if (item.BlendMode != lastItem.BlendMode)
				return false;

			if ((item.DrawTextBorder || lastItem.DrawTextBorder) && textureChanged)
				return false;

			if ((item.MaskTexture != nullptr || lastItem.MaskTexture != nullptr) && (textureChanged || textureMaskChanged))
				return false;

			if (item.CheckerboardSize.has_value() != lastItem.CheckerboardSize.has_value())
				return false;

			return true;
		}

		void InternalCreateBatchesFromItems()
		{
			assert(!BatchItems.empty());

			Batches.emplace_back(0, 1).Textures[0] = BatchItems.front().Texture;
			Vertices.front().SetTextureIndices(0);

			for (u16 i = 1; i < static_cast<u16>(BatchItems.size()); i++)
			{
				const auto& item = BatchItems[i];
				const auto& lastItem = BatchItems[Batches.back().Index];

				if (int availableTextureIndex = FindAvailableTextureSlot(Batches.back(), item.Texture); availableTextureIndex >= 0)
				{
					if (!AreBatchItemsCompatible(item, lastItem))
					{
						Batches.emplace_back(i, 1);
						availableTextureIndex = 0;
					}
					else
					{
						Batches.back().Count++;
					}

					Vertices[i].SetTextureIndices(availableTextureIndex);
					Batches.back().Textures[availableTextureIndex] = item.Texture;
				}
				else
				{
					Batches.emplace_back(i, 1);

					Vertices[i].SetTextureIndices(0);
					Batches.back().Textures[0] = item.Texture;
				}

				assert(GetUsedSpriteTextureSlotsCount(Batches.back()) > 0);
			}
		}

		const D3D11::ShaderPair& GetBatchItemShader(const Detail::SpriteBatchItem& item, int usedTextureSlots) const
		{
			if (item.DrawTextBorder)
				return Shaders.Single.TextureFont;

			if (item.MaskTexture != nullptr)
				return (item.BlendMode == AetBlendMode::Multiply) ? Shaders.Single.TextureMaskMultiply : Shaders.Single.TextureMask;

			if (item.CheckerboardSize.has_value())
				return Shaders.Single.TextureCheckerboard;

			assert(usedTextureSlots > 0 && (usedTextureSlots - 1) < Shaders.Multi.TextureBatch.size());
			if (item.BlendMode == AetBlendMode::Multiply)
				return Shaders.Multi.TextureBatchMultiply;
			else
				return Shaders.Multi.TextureBatch[usedTextureSlots - 1];
		}

		int GetUsedSpriteTextureSlotsCount(const Detail::SpriteBatch& batch) const
		{
			for (int i = 0; i < static_cast<int>(MaxSpriteTextureSlots); i++)
			{
				if (batch.Textures[i] == nullptr)
					return i;
			}

			return MaxSpriteTextureSlots;
		}

		void InternalFlushRenderBatches()
		{
			if (BatchItems.empty())
				return;

			InternalCreateBatchesFromItems();

			VertexBuffer->UploadData(Vertices.size() * sizeof(Detail::SpriteVertices), Vertices.data());

			OrthographicCamera->UpdateMatrices();
			CameraConstantBuffer.Data.ViewProjection = glm::transpose(OrthographicCamera->GetViewProjection());
			CameraConstantBuffer.UploadData();

			const D3D11::ShaderPair* lastShader = nullptr;
			AetBlendMode lastBlendMode = AetBlendMode::Count;

			DefaultTextureSampler.Bind(0);

			for (u16 i = 0; i < Batches.size(); i++)
			{
				const Detail::SpriteBatch& batch = Batches[i];
				const Detail::SpriteBatchItem& item = BatchItems[batch.Index];

				if (lastBlendMode != item.BlendMode)
				{
					InternalSetBlendMode(item.BlendMode);
					lastBlendMode = item.BlendMode;
				}

				const auto usedTextureSlots = GetUsedSpriteTextureSlotsCount(batch);
				if (const auto& itemShader = GetBatchItemShader(item, usedTextureSlots); lastShader != &itemShader)
				{
					itemShader.Bind();
					lastShader = &itemShader;
				}

				if (item.DrawTextBorder)
				{
					std::array<const D3D11::ShaderResourceView*, 1> textureResourceViews =
					{
						item.Texture,
					};
					D3D11::ShaderResourceView::BindArray(0, textureResourceViews);
				}
				else if (item.MaskTexture != nullptr)
				{
					std::array<const D3D11::ShaderResourceView*, 2> textureResourceViews =
					{
						item.Texture,
						item.MaskTexture,
					};
					D3D11::ShaderResourceView::BindArray(0, textureResourceViews);
				}
				else // NOTE: Multi texture batch
				{
					std::array<ID3D11ShaderResourceView*, MaxSpriteTextureSlots> textureResourceViews;
					for (size_t i = 0; i < usedTextureSlots; i++)
						textureResourceViews[i] = (batch.Textures[i] != nullptr) ? batch.Textures[i]->GetResourceView() : nullptr;
					D3D11::D3D.Context->PSSetShaderResources(0, static_cast<UINT>(usedTextureSlots), textureResourceViews.data());
				}

				SpriteConstantBuffer.Data.BlendMode = item.BlendMode;
				SpriteConstantBuffer.Data.Format = item.Texture->GetTextureFormat();
				SpriteConstantBuffer.Data.MaskFormat = (item.MaskTexture == nullptr) ? TextureFormat::Unknown : item.MaskTexture->GetTextureFormat();
				SpriteConstantBuffer.Data.FormatFlags = 0;
				for (u32 i = 0; i < static_cast<u32>(MaxSpriteTextureSlots); i++)
				{
					const bool isYCbCr = (batch.Textures[i] != nullptr) && (batch.Textures[i]->GetTextureFormat() == TextureFormat::RGTC2);
					SpriteConstantBuffer.Data.FormatFlags |= (static_cast<u32>(isYCbCr) << i);
				}
				SpriteConstantBuffer.Data.CheckerboardSize = vec4(item.CheckerboardSize.value_or(vec2(0.0f, 0.0f)), 0.0f, 0.0f);

				SpriteConstantBuffer.UploadData();

				D3D11::D3D.Context->DrawIndexed(
					batch.Count * Detail::SpriteIndices::GetIndexCount(),
					batch.Index * Detail::SpriteIndices::GetIndexCount(),
					0);

				DrawCallCount++;
			}

			InternalClearItems();
		}

		// NOTE: This assumes that the D3D11 context does **not** change externally between the Begin() / End() call
		void InternalSetBeginState()
		{
			RenderTarget->Main.ResizeIfDifferent(RenderTarget->Param.Resolution);
			RenderTarget->Main.SetMultiSampleCountIfDifferent(RenderTarget->Param.MultiSampleCount);
			RenderTarget->Main.BindSetViewport();

			if (RenderTarget->Param.Clear)
				RenderTarget->Main.Clear(RenderTarget->Param.ClearColor);

			RasterizerState.Bind();
			D3D11::D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			InputLayout->Bind();
			VertexBuffer->Bind();
			IndexBuffer->Bind();

			CameraConstantBuffer.BindVertexShader();
			SpriteConstantBuffer.BindPixelShader();
		}

		void InternalSetEndState()
		{
			SpriteConstantBuffer.UnBindPixelShader();
			CameraConstantBuffer.UnBindVertexShader();

			IndexBuffer->UnBind();
			VertexBuffer->UnBind();
			InputLayout->UnBind();

			RasterizerState.UnBind();
			RenderTarget->Main.UnBind();

			if (RenderTarget->Param.MultiSampleCount > 1)
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
			if (BatchItems.size() >= Renderer2D::Impl::MaxBatchItemSize)
				InternalFlushRenderBatches();

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

		void InternalDraw(const RenderCommand2D& command, bool alwaysSetTexCoords = false)
		{
			Detail::SpriteBatchPair pair = InternalCheckFlushAddItem();

			pair.Item->Texture = (command.Texture != nullptr) ? D3D11::GetTexture2D(*command.Texture) : &WhiteTexture;
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
				command.CornerColors.data(),
				(command.Texture != nullptr) || alwaysSetTexCoords);
		}

		void InternalDraw(const RenderCommand2D& command, const RenderCommand2D& commandMask)
		{
			Detail::SpriteBatchPair pair = InternalCheckFlushAddItem();

			pair.Item->Texture = (command.Texture != nullptr) ? D3D11::GetTexture2D(*command.Texture) : &WhiteTexture;
			pair.Item->MaskTexture = (commandMask.Texture != nullptr) ? D3D11::GetTexture2D(*commandMask.Texture) : &WhiteTexture;
			pair.Item->BlendMode = command.BlendMode;
			pair.Item->DrawTextBorder = command.DrawTextBorder;

			pair.Vertices->SetValues(
				commandMask.Position,
				commandMask.SourceRegion,
				TextureOrRegionSize(D3D11::GetTexture2D(commandMask.Texture), commandMask.SourceRegion),
				-commandMask.Origin,
				commandMask.Rotation,
				commandMask.Scale,
				commandMask.CornerColors.data(),
				commandMask.Texture != nullptr);

			// TODO: Null mask texture (?)
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
		impl->InternalSetBeginState();
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
		command.Origin = vec2(0.0f, thickness / 2.0f);
		command.Position = start;
		command.Rotation = glm::degrees(glm::atan(edge.y, edge.x));
		command.SourceRegion = vec4(0.0f, 0.0f, glm::distance(start, end), thickness);
		command.CornerColors = { color, color, color, color };
		impl->InternalDraw(command);
	}

	void Renderer2D::DrawLine(vec2 start, float angle, float length, const vec4& color, float thickness)
	{
		RenderCommand2D command;
		command.Origin = vec2(0.0f, thickness / 2.0f);
		command.Position = start;
		command.Rotation = angle;
		command.SourceRegion = vec4(0.0f, 0.0f, length, thickness);
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
		command.Origin = origin;
		command.Position = position;
		command.Rotation = rotation;
		command.Scale = scale;
		command.SourceRegion = vec4(0.0f, 0.0f, size);
		command.CornerColors = { color, color, color, color };
		impl->InternalDraw(command, true);
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

		impl->InternalFlushRenderBatches();
		impl->InternalSetEndState();

		impl->OrthographicCamera = nullptr;
		impl->RenderTarget = nullptr;
	}
}
