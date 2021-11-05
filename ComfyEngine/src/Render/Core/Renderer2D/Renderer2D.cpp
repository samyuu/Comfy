#include "Renderer2D.h"
#include "Detail/RenderTarget2DImpl.h"
#include "Detail/SpriteBatchData.h"
#include "Detail/TextureSamplerCache.h"
#include "Render/D3D11/D3D11.h"
#include "Render/D3D11/D3D11Buffer.h"
#include "Render/D3D11/D3D11GraphicsTypeHelpers.h"
#include "Render/D3D11/D3D11OpaqueResource.h"
#include "Render/D3D11/D3D11Shader.h"
#include "Render/D3D11/D3D11State.h"
#include "Render/D3D11/D3D11Texture.h"
#include "Render/D3D11/Shader/Bytecode/ShaderBytecode.h"

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

		struct PostProcessData
		{
			vec4 PostProcessParam;
			vec4 PostProcessCoefficients[4];
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
				std::array<D3D11ShaderPair, MaxSpriteTextureSlots> TextureBatch =
				{
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_01_PS(), "Renderer2D::SpriteMultiTextureBatch_01_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_02_PS(), "Renderer2D::SpriteMultiTextureBatch_02_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_03_PS(), "Renderer2D::SpriteMultiTextureBatch_03_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_04_PS(), "Renderer2D::SpriteMultiTextureBatch_04_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_05_PS(), "Renderer2D::SpriteMultiTextureBatch_05_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_06_PS(), "Renderer2D::SpriteMultiTextureBatch_06_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_07_PS(), "Renderer2D::SpriteMultiTextureBatch_07_PS" },
					D3D11ShaderPair { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatch_08_PS(), "Renderer2D::SpriteMultiTextureBatch_08_PS" },
				};
				D3D11ShaderPair TextureBatchMultiply = { GlobalD3D11, SpriteMultiTexture_VS(), SpriteMultiTextureBatchBlend_08_PS(), "Renderer2D::SpriteMultiTextureBatchBlend_08" };
			} Multi;

			struct SingleTextureShaders
			{
				D3D11ShaderPair TextureCheckerboard = { GlobalD3D11, SpriteSingleTexture_VS(), SpriteSingleTextureCheckerboard_PS(), "Renderer2D::SpriteSingleTextureCheckerboard" };
				D3D11ShaderPair TextureFont = { GlobalD3D11, SpriteSingleTexture_VS(), SpriteSingleTextureFont_PS(), "Renderer2D::SpriteSingleTextureFont" };
				D3D11ShaderPair TextureMask = { GlobalD3D11, SpriteSingleTexture_VS(), SpriteSingleTextureMask_PS(), "Renderer2D::SpriteSingleTextureMask" };
				D3D11ShaderPair TextureMaskMultiply = { GlobalD3D11, SpriteSingleTexture_VS(), SpriteSingleTextureMaskBlend_PS(), "Renderer2D::SpriteSingleTextureMaskBlend" };
			} Single;

			struct PostProcessingShaders
			{
				D3D11ShaderPair ColorCorrection = { GlobalD3D11, SpriteFullscreenQuad_VS(), SpriteColorCorrection_PS(), "Renderer2D::SpriteColorCorrection" };
			} PostProcessing;
		} Shaders;

		D3D11ConstantBufferTemplate<CameraConstantData> CameraConstantBuffer = { GlobalD3D11, 0, D3D11_USAGE_DEFAULT };
		D3D11ConstantBufferTemplate<SpriteConstantData> SpriteConstantBuffer = { GlobalD3D11, 0, D3D11_USAGE_DYNAMIC };
		D3D11ConstantBufferTemplate<PostProcessData> PostProcessConstantBuffer = { GlobalD3D11, 1, D3D11_USAGE_DYNAMIC };

		std::unique_ptr<D3D11IndexBuffer> SpriteQuadIndexBuffer = nullptr;
		std::unique_ptr<D3D11VertexBuffer> SpriteQuadVertexBuffer = nullptr;
		std::unique_ptr<D3D11VertexBuffer> SpriteShapeVertexBuffer = nullptr;

		std::unique_ptr<D3D11InputLayout> InputLayout = nullptr;

		// NOTE: Disable backface culling for negatively scaled sprites
		D3D11RasterizerState RasterizerState = { GlobalD3D11, D3D11_FILL_SOLID, D3D11_CULL_NONE };

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
			D3D11BlendState Normal = { GlobalD3D11, AetBlendMode::Normal };
			D3D11BlendState Add = { GlobalD3D11,AetBlendMode::Add };
			D3D11BlendState Multiply = { GlobalD3D11,AetBlendMode::Multiply };
			D3D11BlendState LinearDodge = { GlobalD3D11,AetBlendMode::LinearDodge };
			D3D11BlendState Overlay = { GlobalD3D11,AetBlendMode::Overlay };
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
			D3D11_SetObjectDebugName(CameraConstantBuffer.Buffer.Buffer.Get(), "Renderer2D::CameraConstantBuffer");
			D3D11_SetObjectDebugName(SpriteConstantBuffer.Buffer.Buffer.Get(), "Renderer2D::SpriteConstantBuffer");
			D3D11_SetObjectDebugName(PostProcessConstantBuffer.Buffer.Buffer.Get(), "Renderer2D::PostProcessConstantBuffer");

			D3D11_SetObjectDebugName(RasterizerState.RasterizerState.Get(), "Renderer2D::RasterizerState");

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

			SpriteQuadIndexBuffer = std::make_unique<D3D11IndexBuffer>(GlobalD3D11, indexData.size() * sizeof(Detail::SpriteQuadIndices), indexData.data(), IndexFormat::U16, D3D11_USAGE_IMMUTABLE);
			D3D11_SetObjectDebugName(SpriteQuadIndexBuffer->Buffer.Get(), "Renderer2D::QuadIndexBuffer");
		}

		void InternalCreateVertexBuffer()
		{
			SpriteQuadVertexBuffer = std::make_unique<D3D11VertexBuffer>(GlobalD3D11, MaxBatchItemSize * sizeof(Detail::SpriteQuadVertices), nullptr, sizeof(Detail::SpriteVertex), D3D11_USAGE_DYNAMIC);
			D3D11_SetObjectDebugName(SpriteQuadVertexBuffer->Buffer.Get(), "Renderer2D::QuadVertexBuffer");
		}

		void InternalCreateInputLayout()
		{
			static constexpr D3D11InputElement elements[] =
			{
				{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(Detail::SpriteVertex, Position)				},
				{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(Detail::SpriteVertex, TextureCoordinates)		},
				{ "TEXCOORD",	1, DXGI_FORMAT_R32G32_FLOAT,	offsetof(Detail::SpriteVertex, TextureMaskCoordinates)	},
				{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	offsetof(Detail::SpriteVertex, Color)					},
				{ "TEXINDEX",	0, DXGI_FORMAT_R32_UINT,		offsetof(Detail::SpriteVertex, TextureIndex)			},
			};

			InputLayout = std::make_unique<D3D11InputLayout>(GlobalD3D11, elements, std::size(elements), Shaders.Multi.TextureBatch[0].VS);
			D3D11_SetObjectDebugName(InputLayout->InputLayout.Get(), "Renderer2D::InputLayout");
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

		const D3D11ShaderPair& GetBatchItemShader(const Detail::SpriteBatchItem& item, int usedTextureSlots) const
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
			return TextureSamplers.GetSampler(texView).SamplerState.Get();
		}

		ID3D11ShaderResourceView* TryGetTextureResourceView(TexSamplerView texView)
		{
			const auto* tex2D = texView ? GetD3D11Texture2D(GlobalD3D11, texView.Texture) : nullptr;
			return (tex2D != nullptr) ? tex2D->TextureView.Get() : nullptr;
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
				if (SpriteShapeVertexBuffer == nullptr || (SpriteShapeVertices.size() * sizeof(Detail::SpriteVertex)) > SpriteShapeVertexBuffer->BufferDesc.ByteWidth)
				{
					// NOTE: Use capacity to automatically leverage the exponential growth behavior of std::vector
					SpriteShapeVertexBuffer = std::make_unique<D3D11VertexBuffer>(
						GlobalD3D11,
						SpriteShapeVertices.capacity() * sizeof(Detail::SpriteVertex),
						SpriteShapeVertices.data(),
						sizeof(Detail::SpriteVertex),
						D3D11_USAGE_DYNAMIC);

					D3D11_SetObjectDebugName(SpriteShapeVertexBuffer->Buffer.Get(), "Renderer2D::ShapeVertexBuffer");
				}
				else
				{
					SpriteShapeVertexBuffer->UploadDataIfDynamic(GlobalD3D11, SpriteShapeVertices.size() * sizeof(Detail::SpriteVertex), SpriteShapeVertices.data());
				}
			}

			SpriteQuadVertexBuffer->UploadDataIfDynamic(GlobalD3D11, SpriteQuadVertices.size() * sizeof(Detail::SpriteQuadVertices), SpriteQuadVertices.data());

			Camera->UpdateMatrices();
			CameraConstantBuffer.Data.ViewProjection = glm::transpose(Camera->GetViewProjection());
			CameraConstantBuffer.UploadData(GlobalD3D11);

			const D3D11ShaderPair* lastBoundShader = nullptr;
			const D3D11VertexBuffer* lastBoundVB = nullptr;

			PrimitiveType lastPrimitive = PrimitiveType::Count;
			AetBlendMode lastBlendMode = AetBlendMode::Count;

			for (u16 i = 0; i < DrawCallBatches.size(); i++)
			{
				const Detail::SpriteDrawCallBatch& batch = DrawCallBatches[i];
				const Detail::SpriteBatchItem& item = BatchItems[batch.ItemIndex];
				assert(item.TexView != nullptr);

				if (lastPrimitive != item.Primitive)
				{
					GlobalD3D11.ImmediateContext->IASetPrimitiveTopology(PrimitiveTypeToD3DTopology(item.Primitive));
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
					itemShader.Bind(GlobalD3D11);
					lastBoundShader = &itemShader;
				}

				if (const auto& itemVB = (item.ShapeVertexCount > 0) ? SpriteShapeVertexBuffer : SpriteQuadVertexBuffer; lastBoundVB != itemVB.get())
				{
					itemVB->Bind(GlobalD3D11);
					lastBoundVB = itemVB.get();
				}

				std::array<ID3D11SamplerState*, MaxSpriteTextureSlots> textureSamplers;
				std::array<ID3D11ShaderResourceView*, MaxSpriteTextureSlots> textureResourceViews;
				if (item.DrawTextBorder)
				{
					textureSamplers[0] = TryGetTextureSampler(item.TexView);
					textureResourceViews[0] = TryGetTextureResourceView(item.TexView);

					GlobalD3D11.ImmediateContext->PSSetSamplers(0, 1, textureSamplers.data());
					GlobalD3D11.ImmediateContext->PSSetShaderResources(0, 1, textureResourceViews.data());
				}
				else if (item.MaskTexView != nullptr)
				{
					textureSamplers[0] = TryGetTextureSampler(item.TexView);
					textureSamplers[1] = TryGetTextureSampler(item.MaskTexView);

					textureResourceViews[0] = TryGetTextureResourceView(item.TexView);
					textureResourceViews[1] = TryGetTextureResourceView(item.MaskTexView);

					GlobalD3D11.ImmediateContext->PSSetSamplers(0, 2, textureSamplers.data());
					GlobalD3D11.ImmediateContext->PSSetShaderResources(0, 2, textureResourceViews.data());
				}
				else // NOTE: Multi texture batch
				{
					for (size_t i = 0; i < usedTextureSlots; i++)
					{
						textureSamplers[i] = TryGetTextureSampler(batch.TexViews[i]);
						textureResourceViews[i] = TryGetTextureResourceView(batch.TexViews[i]);
					}

					GlobalD3D11.ImmediateContext->PSSetSamplers(0, static_cast<UINT>(usedTextureSlots), textureSamplers.data());
					GlobalD3D11.ImmediateContext->PSSetShaderResources(0, static_cast<UINT>(usedTextureSlots), textureResourceViews.data());
				}

				SpriteConstantBuffer.Data.BlendMode = item.BlendMode;
				SpriteConstantBuffer.Data.Format = item.TexView.Texture->GetFormat();
				SpriteConstantBuffer.Data.MaskFormat = (item.MaskTexView) ? item.MaskTexView.Texture->GetFormat() : TextureFormat::Unknown;
				SpriteConstantBuffer.Data.FormatFlags = 0;
				for (u32 i = 0; i < static_cast<u32>(MaxSpriteTextureSlots); i++)
					SpriteConstantBuffer.Data.FormatFlags |= (static_cast<u32>(TryGetIsTexViewYCbCr(batch.TexViews[i])) << i);
				SpriteConstantBuffer.Data.CheckerboardSize = vec4(item.CheckerboardSize, 0.0f, 0.0f);
				SpriteConstantBuffer.UploadData(GlobalD3D11);

				if (item.ShapeVertexCount > 0)
				{
					u32 totalVertices = 0;
					for (size_t i = 0; i < batch.ItemCount; i++)
						totalVertices += BatchItems[batch.ItemIndex + i].ShapeVertexCount;

					GlobalD3D11.ImmediateContext->Draw(
						totalVertices,
						item.ShapeVertexIndex);
				}
				else
				{
					GlobalD3D11.ImmediateContext->DrawIndexed(
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
			RenderTarget->Main.RecreateWithNewSizeIfDifferent(GlobalD3D11, RenderTarget->Param.Resolution);
			RenderTarget->Main.RecreateWithNewMultiSampleCountIfDifferent(GlobalD3D11, RenderTarget->Param.MultiSampleCount);
			RenderTarget->Main.BindAndSetViewport(GlobalD3D11);

			RenderTarget->Output.RecreateWithNewSizeIfDifferent(GlobalD3D11, RenderTarget->Param.Resolution);

			if (RenderTarget->Param.Clear)
				RenderTarget->Main.ClearColor(GlobalD3D11, RenderTarget->Param.ClearColor);

			RasterizerState.Bind(GlobalD3D11);
			InputLayout->Bind(GlobalD3D11);
			SpriteQuadIndexBuffer->Bind(GlobalD3D11);

			CameraConstantBuffer.BindVertexShader(GlobalD3D11);
			SpriteConstantBuffer.BindPixelShader(GlobalD3D11);
			PostProcessConstantBuffer.BindPixelShader(GlobalD3D11);
			D3D11_EndDebugEvent();
		}

		void InternalSetEndState()
		{
			D3D11_BeginDebugEvent("Set End State");
			RenderTarget->Main.UnBind(GlobalD3D11);

			if (RenderTarget->Param.MultiSampleCount > 1)
			{
				// TODO: Allow both MSAA and PP
				GlobalD3D11.ImmediateContext->ResolveSubresource(RenderTarget->Output.ColorTexture.Get(), 0, RenderTarget->Main.ColorTexture.Get(), 0, RenderTarget->Main.ColorTextureDesc.Format);
			}
			else if (RenderTarget->Param.PostProcessingEnabled)
			{
				PostProcessConstantBuffer.Data.PostProcessParam[0] = RenderTarget->Param.PostProcessing.Gamma;
				PostProcessConstantBuffer.Data.PostProcessParam[1] = RenderTarget->Param.PostProcessing.Contrast;
				PostProcessConstantBuffer.Data.PostProcessCoefficients[0] = vec4(RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[0], 0.0f);
				PostProcessConstantBuffer.Data.PostProcessCoefficients[1] = vec4(RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[1], 0.0f);
				PostProcessConstantBuffer.Data.PostProcessCoefficients[2] = vec4(RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[2], 0.0f);
				PostProcessConstantBuffer.UploadData(GlobalD3D11);

				Shaders.PostProcessing.ColorCorrection.Bind(GlobalD3D11);

				RenderTarget->Output.Bind(GlobalD3D11);
				GlobalD3D11.ImmediateContext->PSSetShaderResources(0, 1, PtrArg<ID3D11ShaderResourceView*>(RenderTarget->Main.ColorTextureView.Get()));
				GlobalD3D11.ImmediateContext->OMSetBlendState(nullptr, nullptr, D3D11_DEFAULT_SAMPLE_MASK);

				GlobalD3D11.ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
				GlobalD3D11.ImmediateContext->Draw(6, 0);

				GlobalD3D11.ImmediateContext->PSSetShaderResources(0, 1, PtrArg<ID3D11ShaderResourceView*>(nullptr));
				RenderTarget->Output.UnBind(GlobalD3D11);

				Shaders.PostProcessing.ColorCorrection.UnBind(GlobalD3D11);
			}
			else
			{
				GlobalD3D11.ImmediateContext->CopyResource(RenderTarget->Output.ColorTexture.Get(), RenderTarget->Main.ColorTexture.Get());
			}

			PostProcessConstantBuffer.UnBindPixelShader(GlobalD3D11);
			SpriteConstantBuffer.UnBindPixelShader(GlobalD3D11);
			CameraConstantBuffer.UnBindVertexShader(GlobalD3D11);

			SpriteQuadIndexBuffer->UnBind(GlobalD3D11);
			SpriteQuadVertexBuffer->UnBind(GlobalD3D11);
			InputLayout->UnBind(GlobalD3D11);
			RasterizerState.UnBind(GlobalD3D11);

			D3D11_EndDebugEvent();
		}

		void InternalSetBlendMode(AetBlendMode blendMode)
		{
			switch (blendMode)
			{
			case AetBlendMode::Unknown:
				AetBlendStates.Normal.UnBind(GlobalD3D11);
				break;
			default:
			case AetBlendMode::Normal:
				AetBlendStates.Normal.Bind(GlobalD3D11);
				break;
			case AetBlendMode::Add:
				AetBlendStates.Add.Bind(GlobalD3D11);
				break;
			case AetBlendMode::Multiply:
				AetBlendStates.Multiply.Bind(GlobalD3D11);
				break;
			case AetBlendMode::LinearDodge:
				AetBlendStates.LinearDodge.Bind(GlobalD3D11);
				break;
			case AetBlendMode::Overlay:
				AetBlendStates.Overlay.Bind(GlobalD3D11);
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
				(command.TexView != nullptr) || alwaysSetTexCoords,
				(command.TexView != nullptr && command.TexView.Texture != nullptr) ? command.TexView.Texture->GPU_FlipY : false);
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
				commandMask.TexView != nullptr,
				(commandMask.TexView != nullptr && commandMask.TexView.Texture != nullptr) ? commandMask.TexView.Texture->GPU_FlipY : false);

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

	void Renderer2D::UploadToGPUFreeCPUMemory(Graphics::SprSet& sprSet)
	{
		UploadToGPUFreeCPUMemory(sprSet.TexSet);
	}

	void Renderer2D::UploadToGPUFreeCPUMemory(Graphics::TexSet& texSet)
	{
		for (auto& tex : texSet.Textures)
		{
			if (tex != nullptr)
				UploadToGPUFreeCPUMemory(*tex);
		}
	}

	void Renderer2D::UploadToGPUFreeCPUMemory(Graphics::Tex& tex)
	{
		const auto* texture2D = GetD3D11Texture2D(GlobalD3D11, tex);
		if (texture2D == nullptr)
			return;

		for (auto& mipMaps : tex.MipMapsArray)
		{
			for (auto& mip : mipMaps)
			{
				mip.DataSize = 0;
				mip.Data = nullptr;
			}
		}
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
