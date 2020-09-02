#include "Renderer2D.h"
#include "Detail/RenderTarget2DImpl.h"
#include "Detail/SpriteBatchData.h"
#include "Detail/TextureSamplerCache.h"
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

	vec2 GetTexViewTextureOrRegionSize(TexSamplerView texView, const vec4& source)
	{
		return (texView) ? vec2(texView.Texture->GetSize()) : vec2(source.z, source.w);
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
		FontRenderer FontRenderer;

		u32 DrawCallCount = 0;

		struct RendererShaderPairs
		{
			struct MultiTextureShaders
			{
				// TODO: Permutation for all texture formats being the same to ~~avoid any branching~~ have a single branch
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

		std::unique_ptr<D3D11::StaticIndexBuffer> SpriteQuadIndexBuffer = nullptr;
		std::unique_ptr<D3D11::DynamicVertexBuffer> SpriteQuadVertexBuffer = nullptr;
		std::unique_ptr<D3D11::DynamicVertexBuffer> SpriteShapeVertexBuffer = nullptr;

		std::unique_ptr<D3D11::InputLayout> InputLayout = nullptr;

		// NOTE: Disable backface culling for negatively scaled sprites
		D3D11::RasterizerState RasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE };

		Detail::TextureSamplerCache2D TextureSamplers = {};

		// NOTE: Avoid additional branches by using a 1x1 white fallback texture for rendering solid color
		const Tex WhiteChipTexture = []
		{
			Tex chip;
			chip.Name = "Renderer2D::WhiteChipTexture";
			auto& mipMap = chip.MipMapsArray.emplace_back().emplace_back();
			mipMap.Size = ivec2(1);
			mipMap.Format = TextureFormat::RGBA8;
			mipMap.DataSize = 1 * sizeof(u32);
			mipMap.Data = std::make_unique<u8[]>(mipMap.DataSize);
			auto* rgbaPixels = reinterpret_cast<u32*>(mipMap.Data.get());
			rgbaPixels[0] = 0xFFFFFFFF;
			return chip;
		}();

		const TexSamplerView WhiteChipTextureView = TexSamplerView(&WhiteChipTexture,
			TextureAddressMode::ClampBorder,
			TextureAddressMode::ClampBorder,
			TextureFilter::Point);

		struct AetBlendStates
		{
			D3D11::BlendState Normal = { AetBlendMode::Normal };
			D3D11::BlendState Add = { AetBlendMode::Add };
			D3D11::BlendState Multiply = { AetBlendMode::Multiply };
			D3D11::BlendState LinearDodge = { AetBlendMode::LinearDodge };
			D3D11::BlendState Overlay = { AetBlendMode::Overlay };
		} AetBlendStates;

		std::vector<Detail::SpriteDrawCallBatch> DrawCallBatches;
		std::vector<Detail::SpriteBatchItem> BatchItems;
		std::vector<Detail::SpriteQuadVertices> SpriteQuadVertices;
		std::vector<Detail::SpriteVertex> SpriteShapeVertices;

		Camera2D* Camera = nullptr;
		Detail::RenderTarget2DImpl* RenderTarget = nullptr;

	public:
		Impl(Renderer2D& parent) : AetRenderer(parent), FontRenderer(parent)
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
			std::array<Detail::SpriteQuadIndices, MaxBatchItemSize> indexData;

			for (u16 i = 0, offset = 0; i < indexData.size(); i++)
			{
				// NOTE: Vertex index order:
				//		 [0] TopLeft	- [1] TopRight
				//		 [2] BottomLeft - [3] BottomRight;
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

				offset += static_cast<u16>(Detail::SpriteQuadVertices::GetVertexCount());
			}

			SpriteQuadIndexBuffer = std::make_unique<D3D11::StaticIndexBuffer>(indexData.size() * sizeof(Detail::SpriteQuadIndices), indexData.data(), IndexFormat::U16);
			D3D11_SetObjectDebugName(SpriteQuadIndexBuffer->GetBuffer(), "Renderer2D::QuadIndexBuffer");
		}

		void InternalCreateVertexBuffer()
		{
			SpriteQuadVertexBuffer = std::make_unique<D3D11::DynamicVertexBuffer>(MaxBatchItemSize * sizeof(Detail::SpriteQuadVertices), nullptr, sizeof(Detail::SpriteVertex));
			D3D11_SetObjectDebugName(SpriteQuadVertexBuffer->GetBuffer(), "Renderer2D::QuadVertexBuffer");
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

		int FindAvailableTextureSlot(Detail::SpriteDrawCallBatch& currentBatch, TexSamplerView texView) const
		{
			for (int i = 0; i < static_cast<int>(MaxSpriteTextureSlots); i++)
			{
				if (!currentBatch.TexViews[i] || currentBatch.TexViews[i] == texView)
					return i;
			}

			return -1;
		}

		bool AreBatchItemsCompatible(const Detail::SpriteBatchItem& item, const Detail::SpriteBatchItem& lastItem) const
		{
			if (item.Primitive != lastItem.Primitive)
				return false;

			if (item.BlendMode != lastItem.BlendMode)
				return false;

			if (item.DrawTextBorder != lastItem.DrawTextBorder)
				return false;

			const bool textureChanged = (item.TexView != lastItem.TexView);
			const bool textureMaskChanged = (item.MaskTexView != lastItem.MaskTexView);

			if ((item.MaskTexView != nullptr || lastItem.MaskTexView != nullptr) && (textureChanged || textureMaskChanged))
				return false;

			if (item.DrawCheckerboard != lastItem.DrawCheckerboard)
				return false;

			if (item.ShapeVertexCount > 0 != lastItem.ShapeVertexCount > 0)
				return false;

			return true;
		}

		void InternalCreateDrawCallBatchesFromItems()
		{
			assert(!BatchItems.empty());

			u16 quadIndex = 0;
			DrawCallBatches.emplace_back(0, 1, quadIndex).TexViews[0] = BatchItems.front().TexView;

			if (BatchItems.front().ShapeVertexCount == 0)
				quadIndex++;

			for (u16 itemIndex = 1; itemIndex < static_cast<u16>(BatchItems.size()); itemIndex++)
			{
				const auto& item = BatchItems[itemIndex];
				const auto& lastItem = BatchItems[DrawCallBatches.back().ItemIndex];

				if (int availableTextureIndex = FindAvailableTextureSlot(DrawCallBatches.back(), item.TexView); availableTextureIndex >= 0)
				{
					if (AreBatchItemsCompatible(item, lastItem))
					{
						DrawCallBatches.back().ItemCount++;
					}
					else
					{
						DrawCallBatches.emplace_back(itemIndex, 1, quadIndex);
						availableTextureIndex = 0;
					}

					if (item.ShapeVertexCount > 0)
					{
						for (size_t i = 0; i < item.ShapeVertexCount; i++)
							SpriteShapeVertices[item.ShapeVertexIndex + i].TextureIndex = availableTextureIndex;

						quadIndex--;
					}
					else
					{
						SpriteQuadVertices[quadIndex].SetTextureIndices(availableTextureIndex);
					}

					DrawCallBatches.back().TexViews[availableTextureIndex] = item.TexView;
				}
				else
				{
					if (item.ShapeVertexCount > 0)
						quadIndex--;

					DrawCallBatches.emplace_back(itemIndex, 1, quadIndex).TexViews[0] = item.TexView;
				}

				quadIndex++;
				assert(GetUsedSpriteTextureSlotsCount(DrawCallBatches.back()) > 0);
			}
		}

		const D3D11::ShaderPair& GetBatchItemShader(const Detail::SpriteBatchItem& item, int usedTextureSlots) const
		{
			if (item.DrawTextBorder)
				return Shaders.Single.TextureFont;

			if (item.MaskTexView != nullptr)
				return (item.BlendMode == AetBlendMode::Multiply) ? Shaders.Single.TextureMaskMultiply : Shaders.Single.TextureMask;

			if (item.DrawCheckerboard)
				return Shaders.Single.TextureCheckerboard;

			assert(usedTextureSlots > 0 && (usedTextureSlots - 1) < Shaders.Multi.TextureBatch.size());
			if (item.BlendMode == AetBlendMode::Multiply)
				return Shaders.Multi.TextureBatchMultiply;
			else
				return Shaders.Multi.TextureBatch[usedTextureSlots - 1];
		}

		int GetUsedSpriteTextureSlotsCount(const Detail::SpriteDrawCallBatch& batch) const
		{
			for (int i = 0; i < static_cast<int>(MaxSpriteTextureSlots); i++)
			{
				if (batch.TexViews[i] == nullptr)
					return i;
			}

			return MaxSpriteTextureSlots;
		}

		ID3D11SamplerState* TryGetTextureSampler(TexSamplerView texView)
		{
			return TextureSamplers.GetSampler(texView).GetSampler();
		}

		ID3D11ShaderResourceView* TryGetTextureResourceView(TexSamplerView texView)
		{
			const auto* tex2D = texView ? D3D11::GetTexture2D(texView.Texture) : nullptr;
			return (tex2D != nullptr) ? tex2D->GetResourceView() : nullptr;
		}

		bool TryGetIsTexViewYCbCr(TexSamplerView texView) const
		{
			return (texView && texView.Texture->GetFormat() == TextureFormat::RGTC2);
		}

		void InternalFlushRenderBatches()
		{
			if (BatchItems.empty())
				return;

			InternalCreateDrawCallBatchesFromItems();
			D3D11_BeginDebugEvent("Render Batches");

			if (!SpriteShapeVertices.empty())
			{
				if (SpriteShapeVertexBuffer == nullptr || (SpriteShapeVertices.size() * sizeof(Detail::SpriteVertex)) > SpriteShapeVertexBuffer->GetDescription().ByteWidth)
				{
					// NOTE: Use capacity to automatically leverage the exponential growth behavior of std::vector
					SpriteShapeVertexBuffer = std::make_unique<D3D11::DynamicVertexBuffer>(
						SpriteShapeVertices.capacity() * sizeof(Detail::SpriteVertex),
						SpriteShapeVertices.data(),
						sizeof(Detail::SpriteVertex));

					D3D11_SetObjectDebugName(SpriteShapeVertexBuffer->GetBuffer(), "Renderer2D::ShapeVertexBuffer");
				}
				else
				{
					SpriteShapeVertexBuffer->UploadData(SpriteShapeVertices.size() * sizeof(Detail::SpriteVertex), SpriteShapeVertices.data());
				}
			}

			SpriteQuadVertexBuffer->UploadData(SpriteQuadVertices.size() * sizeof(Detail::SpriteQuadVertices), SpriteQuadVertices.data());

			Camera->UpdateMatrices();
			CameraConstantBuffer.Data.ViewProjection = glm::transpose(Camera->GetViewProjection());
			CameraConstantBuffer.UploadData();

			const D3D11::ShaderPair* lastBoundShader = nullptr;
			const D3D11::DynamicVertexBuffer* lastBoundVB = nullptr;

			PrimitiveType lastPrimitive = PrimitiveType::Count;
			AetBlendMode lastBlendMode = AetBlendMode::Count;

			for (u16 i = 0; i < DrawCallBatches.size(); i++)
			{
				const Detail::SpriteDrawCallBatch& batch = DrawCallBatches[i];
				const Detail::SpriteBatchItem& item = BatchItems[batch.ItemIndex];
				assert(item.TexView != nullptr);

				if (lastPrimitive != item.Primitive)
				{
					D3D11::D3D.Context->IASetPrimitiveTopology(D3D11::PrimitiveTypeToD3DTopology(item.Primitive));
					lastPrimitive = item.Primitive;
				}

				if (lastBlendMode != item.BlendMode)
				{
					InternalSetBlendMode(item.BlendMode);
					lastBlendMode = item.BlendMode;
				}

				const auto usedTextureSlots = GetUsedSpriteTextureSlotsCount(batch);
				if (const auto& itemShader = GetBatchItemShader(item, usedTextureSlots); lastBoundShader != &itemShader)
				{
					itemShader.Bind();
					lastBoundShader = &itemShader;
				}

				if (const auto& itemVB = (item.ShapeVertexCount > 0) ? SpriteShapeVertexBuffer : SpriteQuadVertexBuffer; lastBoundVB != itemVB.get())
				{
					itemVB->Bind();
					lastBoundVB = itemVB.get();
				}

				std::array<ID3D11SamplerState*, MaxSpriteTextureSlots> textureSamplers;
				std::array<ID3D11ShaderResourceView*, MaxSpriteTextureSlots> textureResourceViews;
				if (item.DrawTextBorder)
				{
					textureSamplers[0] = TryGetTextureSampler(item.TexView);
					textureResourceViews[0] = TryGetTextureResourceView(item.TexView);

					D3D11::D3D.Context->PSSetSamplers(0, 1, textureSamplers.data());
					D3D11::D3D.Context->PSSetShaderResources(0, 1, textureResourceViews.data());
				}
				else if (item.MaskTexView != nullptr)
				{
					textureSamplers[0] = TryGetTextureSampler(item.TexView);
					textureSamplers[1] = TryGetTextureSampler(item.MaskTexView);

					textureResourceViews[0] = TryGetTextureResourceView(item.TexView);
					textureResourceViews[1] = TryGetTextureResourceView(item.MaskTexView);

					D3D11::D3D.Context->PSSetSamplers(0, 2, textureSamplers.data());
					D3D11::D3D.Context->PSSetShaderResources(0, 2, textureResourceViews.data());
				}
				else // NOTE: Multi texture batch
				{
					for (size_t i = 0; i < usedTextureSlots; i++)
					{
						textureSamplers[i] = TryGetTextureSampler(batch.TexViews[i]);
						textureResourceViews[i] = TryGetTextureResourceView(batch.TexViews[i]);
					}

					D3D11::D3D.Context->PSSetSamplers(0, static_cast<UINT>(usedTextureSlots), textureSamplers.data());
					D3D11::D3D.Context->PSSetShaderResources(0, static_cast<UINT>(usedTextureSlots), textureResourceViews.data());
				}

				SpriteConstantBuffer.Data.BlendMode = item.BlendMode;
				SpriteConstantBuffer.Data.Format = item.TexView.Texture->GetFormat();
				SpriteConstantBuffer.Data.MaskFormat = (item.MaskTexView) ? item.MaskTexView.Texture->GetFormat() : TextureFormat::Unknown;
				SpriteConstantBuffer.Data.FormatFlags = 0;
				for (u32 i = 0; i < static_cast<u32>(MaxSpriteTextureSlots); i++)
					SpriteConstantBuffer.Data.FormatFlags |= (static_cast<u32>(TryGetIsTexViewYCbCr(batch.TexViews[i])) << i);
				SpriteConstantBuffer.Data.CheckerboardSize = vec4(item.CheckerboardSize, 0.0f, 0.0f);
				SpriteConstantBuffer.UploadData();

				if (item.ShapeVertexCount > 0)
				{
					u32 totalVertices = 0;
					for (size_t i = 0; i < batch.ItemCount; i++)
						totalVertices += BatchItems[batch.ItemIndex + i].ShapeVertexCount;

					D3D11::D3D.Context->Draw(
						totalVertices,
						item.ShapeVertexIndex);
				}
				else
				{
					D3D11::D3D.Context->DrawIndexed(
						batch.ItemCount * Detail::SpriteQuadIndices::TotalIndices(),
						batch.QuadIndex * Detail::SpriteQuadIndices::TotalIndices(),
						0);
				}

				DrawCallCount++;
			}

			D3D11_EndDebugEvent();
			InternalClearItems();
		}

		// NOTE: This assumes that the D3D11 context does **not** change externally between the Begin() / End() call
		void InternalSetBeginState()
		{
			D3D11_BeginDebugEvent("Set Begin State");
			RenderTarget->Main.ResizeIfDifferent(RenderTarget->Param.Resolution);
			RenderTarget->Main.SetMultiSampleCountIfDifferent(RenderTarget->Param.MultiSampleCount);
			RenderTarget->Main.BindSetViewport();

			if (RenderTarget->Param.Clear)
				RenderTarget->Main.Clear(RenderTarget->Param.ClearColor);

			RasterizerState.Bind();
			InputLayout->Bind();
			SpriteQuadIndexBuffer->Bind();

			CameraConstantBuffer.BindVertexShader();
			SpriteConstantBuffer.BindPixelShader();
			D3D11_EndDebugEvent();
		}

		void InternalSetEndState()
		{
			D3D11_BeginDebugEvent("Set End State");
			SpriteConstantBuffer.UnBindPixelShader();
			CameraConstantBuffer.UnBindVertexShader();

			SpriteQuadIndexBuffer->UnBind();
			SpriteQuadVertexBuffer->UnBind();
			InputLayout->UnBind();

			RasterizerState.UnBind();
			RenderTarget->Main.UnBind();

			if (RenderTarget->Param.MultiSampleCount > 1)
			{
				RenderTarget->ResolvedMain.ResizeIfDifferent(RenderTarget->Param.Resolution);
				D3D11::D3D.Context->ResolveSubresource(RenderTarget->ResolvedMain.GetResource(), 0, RenderTarget->Main.GetResource(), 0, RenderTarget->Main.GetBackBufferDescription().Format);
			}
			D3D11_EndDebugEvent();
		}

		void InternalSetBlendMode(AetBlendMode blendMode)
		{
			switch (blendMode)
			{
			case AetBlendMode::Unknown:
				AetBlendStates.Normal.UnBind();
				break;
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

		Detail::SpriteBatchItemVertexView InternalCheckFlushAddSpriteQuadAndItem()
		{
			if (BatchItems.size() >= Renderer2D::Impl::MaxBatchItemSize)
				InternalFlushRenderBatches();

			return { &BatchItems.emplace_back(), &SpriteQuadVertices.emplace_back() };
		}

		void InternalClearItems()
		{
			DrawCallBatches.clear();
			BatchItems.clear();
			SpriteQuadVertices.clear();
			SpriteShapeVertices.clear();
		}

		void InternalDraw(const RenderCommand2D& command, bool alwaysSetTexCoords = false)
		{
			Detail::SpriteBatchItemVertexView pair = InternalCheckFlushAddSpriteQuadAndItem();

			pair.Item->TexView = (command.TexView) ? command.TexView : WhiteChipTextureView;
			pair.Item->MaskTexView = nullptr;
			pair.Item->Primitive = PrimitiveType::Triangles;
			pair.Item->BlendMode = command.BlendMode;
			pair.Item->DrawTextBorder = command.DrawTextBorder;

			pair.Vertices->SetValues(
				command.Position,
				command.SourceRegion,
				GetTexViewTextureOrRegionSize(command.TexView, command.SourceRegion),
				-command.Origin,
				command.Rotation,
				command.Scale,
				command.CornerColors.data(),
				(command.TexView != nullptr) || alwaysSetTexCoords);
		}

		void InternalDraw(const RenderCommand2D& command, const RenderCommand2D& commandMask)
		{
			Detail::SpriteBatchItemVertexView pair = InternalCheckFlushAddSpriteQuadAndItem();

			pair.Item->TexView = (command.TexView) ? command.TexView : WhiteChipTextureView;
			pair.Item->MaskTexView = (commandMask.TexView) ? commandMask.TexView : WhiteChipTextureView;
			pair.Item->Primitive = PrimitiveType::Triangles;
			pair.Item->BlendMode = command.BlendMode;
			pair.Item->DrawTextBorder = command.DrawTextBorder;

			pair.Vertices->SetValues(
				commandMask.Position,
				commandMask.SourceRegion,
				GetTexViewTextureOrRegionSize(commandMask.TexView, commandMask.SourceRegion),
				-commandMask.Origin,
				commandMask.Rotation,
				commandMask.Scale,
				commandMask.CornerColors.data(),
				commandMask.TexView != nullptr);

			// TODO: Null mask texture (?)
			pair.Vertices->SetTexMaskCoords(
				command.TexView,
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

		void InternalDrawVertices(const PositionTextureColorVertex* vertices, size_t vertexCount, TexSamplerView texView, AetBlendMode blendMode, PrimitiveType primitive)
		{
			assert(vertices != nullptr && vertexCount > 0 && vertexCount < std::numeric_limits<u16>::max());

			if (BatchItems.size() >= Renderer2D::Impl::MaxBatchItemSize)
				InternalFlushRenderBatches();

			auto& batchItem = BatchItems.emplace_back();
			batchItem.TexView = (texView) ? texView : WhiteChipTextureView;
			batchItem.Primitive = primitive;
			batchItem.BlendMode = blendMode;
			batchItem.ShapeVertexIndex = static_cast<u16>(SpriteShapeVertices.size());
			batchItem.ShapeVertexCount = static_cast<u16>(vertexCount);

			SpriteShapeVertices.reserve(SpriteShapeVertices.size() + vertexCount);
			for (size_t i = 0; i < vertexCount; i++)
			{
				const auto& sourceVertex = vertices[i];
				auto& spriteVertex = SpriteShapeVertices.emplace_back();

				spriteVertex.Position = sourceVertex.Position;
				spriteVertex.TextureCoordinates = sourceVertex.TextureCoordinates;
				spriteVertex.Color = Detail::ColorVec4ToU32(sourceVertex.Color);
			}
		}
	};

	Renderer2D::Renderer2D() : impl(std::make_unique<Impl>(*this))
	{
	}

	Renderer2D::~Renderer2D()
	{
	}

	void Renderer2D::Begin(Camera2D& camera, RenderTarget2D& renderTarget)
	{
		assert(impl->Camera == nullptr);
		D3D11_BeginDebugEvent("Renderer2D::Begin - End");

		impl->DrawCallCount = 0;
		impl->Camera = &camera;
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

		impl->BatchItems.back().DrawCheckerboard = true;
		impl->BatchItems.back().CheckerboardSize = (size * scale * precision);
	}

	void Renderer2D::DrawVertices(const PositionTextureColorVertex* vertices, size_t vertexCount, TexSamplerView texView, AetBlendMode blendMode, PrimitiveType primitive)
	{
		if (vertices == nullptr || vertexCount == 0)
			return;

		impl->InternalDrawVertices(vertices, vertexCount, texView, blendMode, primitive);
	}

	AetRenderer& Renderer2D::Aet()
	{
		return impl->AetRenderer;
	}

	FontRenderer& Renderer2D::Font()
	{
		return impl->FontRenderer;
	}

	const Camera2D& Renderer2D::GetCamera() const
	{
		assert(impl->Camera != nullptr);
		return *impl->Camera;
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
		assert(impl->Camera != nullptr);

		impl->InternalFlushRenderBatches();
		impl->InternalSetEndState();

		impl->Camera = nullptr;
		impl->RenderTarget = nullptr;
		D3D11_EndDebugEvent();
	}
}
