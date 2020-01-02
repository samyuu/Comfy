#include "ComfyD3D11.h"
#include "Graphics/Direct3D/Direct3D.h"
#include "Graphics/Direct3D/D3D_BlendState.h"
#include "Graphics/Direct3D/D3D_ConstantBuffer.h"
#include "Graphics/Direct3D/D3D_DepthBuffer.h"
#include "Graphics/Direct3D/D3D_DepthStencilState.h"
#include "Graphics/Direct3D/D3D_IndexBuffer.h"
#include "Graphics/Direct3D/D3D_InputLayout.h"
#include "Graphics/Direct3D/D3D_RasterizerState.h"
#include "Graphics/Direct3D/D3D_Texture.h"
#include "Graphics/Direct3D/D3D_TextureSampler.h"
#include "Graphics/Direct3D/D3D_VertexBuffer.h"
#include "Graphics/Direct3D/ShaderBytecode/ShaderBytecode.h"

using namespace Graphics;

namespace ImGui
{
	namespace ComfyD3D11PlatformInterface
	{
		void PlatformCreateWindow(ImGuiViewport* viewport);
		void PlatformDestroyWindow(ImGuiViewport* viewport);
		void PlatformSetWindowSize(ImGuiViewport* viewport, ImVec2 size);
		void PlatformRenderWindow(ImGuiViewport* viewport, void*);
		void PlatformSwapBuffers(ImGuiViewport* viewport, void*);
	}

	namespace ComfyD3D11
	{
		struct MatrixConstantData
		{
			mat4 ModelViewProjection;
		};

		struct DynamicConstantData
		{
			int RenderCubeMap;
			int CubeMapFace;
			int CubeMapUnwrapNet;
			int DecompressRGTC;
		};

		struct ComfyD3D11DeviceObjects
		{
			UniquePtr<D3D_DynamicVertexBuffer> VertexBuffer = nullptr;
			UniquePtr<D3D_DynamicIndexBuffer> IndexBuffer = nullptr;

			D3D_ShaderPair DefaultShader = { ImGuiDefault_VS(), ImGuiDefault_PS(), "ComfyD3D11::ImGuiDefault" };
			D3D_ShaderPair CustomShader = { ImGuiDefault_VS(), ImGuiCustom_PS(), "ComfyD3D11::ImGuiCustom" };

			D3D_DefaultConstantBufferTemplate<MatrixConstantData> MatrixCB = { 0, "ComfyD3D11::MatrixCB" };
			D3D_DynamicConstantBufferTemplate<DynamicConstantData> DynamicCB = { 0, "ComfyD3D11::DynamicCB" };

			UniquePtr<D3D_InputLayout> InputLayout = nullptr;

			D3D_TextureSampler FontSampler = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP };
			UniquePtr<D3D_Texture2D> FontTexture = nullptr;

			D3D_BlendState BlendState = { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ZERO };
			D3D_RasterizerState RasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE, true, "ComfyD3D11::RasterizerState" };
			D3D_DepthStencilState DepthStencilState = { false, D3D11_DEPTH_WRITE_MASK_ALL, "ComfyD3D11::DepthStencilState" };

			ComfyD3D11DeviceObjects()
			{
				static constexpr InputElement elements[] =
				{
					{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(ImDrawVert, pos)	},
					{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(ImDrawVert, uv)	},
					{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	offsetof(ImDrawVert, col)	},
				};

				InputLayout = MakeUnique<D3D_InputLayout>(elements, std::size(elements), DefaultShader.VS);
				D3D_SetObjectDebugName(InputLayout->GetLayout(), "ComfyD3D11::InputLayout");

				D3D_SetObjectDebugName(FontSampler.GetSampler(), "ComfyD3D11::FontSampler");
				D3D_SetObjectDebugName(BlendState.GetBlendState(), "ComfyD3D11::BlendState");

				uint8_t* rgbaPixels;
				ivec2 textureSize;
				GetIO().Fonts->GetTexDataAsRGBA32(&rgbaPixels, &textureSize.x, &textureSize.y);

				FontTexture = MakeUnique<D3D_Texture2D>(textureSize, reinterpret_cast<uint32_t*>(rgbaPixels));
				D3D_SetObjectDebugName(FontTexture->GetTexture(), "ComfyD3D11::FontTexture");
				D3D_SetObjectDebugName(FontTexture->GetResourceView(), "ComfyD3D11::FontTextureView");

				GetIO().Fonts->TexID = *FontTexture;
			}

			~ComfyD3D11DeviceObjects()
			{
				GetIO().Fonts->TexID = 0;
			}
		};

		struct ComfyD3D11Data
		{
			ComPtr<IDXGIFactory> Factory = nullptr;

			UniquePtr<ComfyD3D11DeviceObjects> DeviceObjects = nullptr;

			int VertexBufferSize = 5000;
			int IndexBufferSize = 10000;

		} Data;

		bool Initialize()
		{
			ImGuiIO& io = GetIO();
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
			io.BackendRendererName = "ComfyD3D11";

			ComPtr<IDXGIDevice> dxgiDevice = nullptr;
			ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;

			if (D3D.Device->QueryInterface(IID_PPV_ARGS(&dxgiDevice)) != S_OK)
				return false;

			if (dxgiDevice->GetParent(IID_PPV_ARGS(&dxgiAdapter)) != S_OK)
				return false;

			if (dxgiAdapter->GetParent(IID_PPV_ARGS(&Data.Factory)) != S_OK)
				return false;

			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				ImGuiPlatformIO& platform_io = GetPlatformIO();
				platform_io.Renderer_CreateWindow = ComfyD3D11PlatformInterface::PlatformCreateWindow;
				platform_io.Renderer_DestroyWindow = ComfyD3D11PlatformInterface::PlatformDestroyWindow;
				platform_io.Renderer_SetWindowSize = ComfyD3D11PlatformInterface::PlatformSetWindowSize;
				platform_io.Renderer_RenderWindow = ComfyD3D11PlatformInterface::PlatformRenderWindow;
				platform_io.Renderer_SwapBuffers = ComfyD3D11PlatformInterface::PlatformSwapBuffers;
			}

			return true;
		}

		void Shutdown()
		{
			DestroyPlatformWindows();

			Data.DeviceObjects = nullptr;
			Data.Factory = nullptr;
		}

		bool CreateDeviceObjects()
		{
			Data.DeviceObjects = MakeUnique<ComfyD3D11DeviceObjects>();
			return true;
		}

		void InvalidateDeviceObjects()
		{
			Data.DeviceObjects = nullptr;
		}

		void NewFrame()
		{
			if (Data.DeviceObjects == nullptr)
				CreateDeviceObjects();
		}

		void RenderDrawData(const ImDrawData* drawData)
		{
			// NOTE: Avoid rendering when minimized
			if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
				return;

			if (Data.DeviceObjects->VertexBuffer == nullptr || Data.VertexBufferSize < drawData->TotalVtxCount)
			{
				Data.VertexBufferSize = drawData->TotalVtxCount + 5000;
				Data.DeviceObjects->VertexBuffer = MakeUnique<D3D_DynamicVertexBuffer>(Data.VertexBufferSize * sizeof(ImDrawVert), nullptr, sizeof(ImDrawVert));

				D3D_SetObjectDebugName(Data.DeviceObjects->VertexBuffer->GetBuffer(), "ComfyD3D11::VertexBuffer");
			}

			if (Data.DeviceObjects->IndexBuffer == nullptr || Data.IndexBufferSize < drawData->TotalIdxCount)
			{
				Data.IndexBufferSize = drawData->TotalIdxCount + 10000;
				Data.DeviceObjects->IndexBuffer = MakeUnique<D3D_DynamicIndexBuffer>(Data.IndexBufferSize * sizeof(ImDrawIdx), nullptr, IndexType::UInt16);

				D3D_SetObjectDebugName(Data.DeviceObjects->IndexBuffer->GetBuffer(), "ComfyD3D11::IndexBuffer");
			}

			// NOTE: Upload vertex / index data into a single contiguous GPU buffer
			{
				D3D11_MAPPED_SUBRESOURCE mappedVertices, mappedIndices;
				D3D.Context->Map(Data.DeviceObjects->VertexBuffer->GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertices);
				D3D.Context->Map(Data.DeviceObjects->IndexBuffer->GetBuffer(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIndices);

				ImDrawVert* vertexDst = static_cast<ImDrawVert*>(mappedVertices.pData);
				ImDrawIdx* indexDst = static_cast<ImDrawIdx*>(mappedIndices.pData);
				for (int i = 0; i < drawData->CmdListsCount; i++)
				{
					const ImDrawList* drawList = drawData->CmdLists[i];

					memcpy(vertexDst, drawList->VtxBuffer.Data, drawList->VtxBuffer.Size * sizeof(ImDrawVert));
					memcpy(indexDst, drawList->IdxBuffer.Data, drawList->IdxBuffer.Size * sizeof(ImDrawIdx));

					vertexDst += drawList->VtxBuffer.Size;
					indexDst += drawList->IdxBuffer.Size;
				}

				D3D.Context->Unmap(Data.DeviceObjects->VertexBuffer->GetBuffer(), 0);
				D3D.Context->Unmap(Data.DeviceObjects->IndexBuffer->GetBuffer(), 0);
			}

			// NOTE: Setup orthographic projection matrix
			{
				const float left = drawData->DisplayPos.x;
				const float right = drawData->DisplayPos.x + drawData->DisplaySize.x;
				const float top = drawData->DisplayPos.y;
				const float bottom = drawData->DisplayPos.y + drawData->DisplaySize.y;

				Data.DeviceObjects->MatrixCB.Data.ModelViewProjection = glm::transpose(glm::ortho(left, right, bottom, top, 0.0f, 1.0f));
				Data.DeviceObjects->MatrixCB.UploadData();
			}

			struct D3D_StateBackup
			{
				UINT ScissorRectsCount, ViewportsCount;
				D3D11_RECT ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				D3D11_VIEWPORT Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				ID3D11RasterizerState* RS;
				ID3D11BlendState* BlendState;
				FLOAT BlendFactor[4];
				UINT SampleMask;
				UINT StencilRef;
				ID3D11DepthStencilState* DepthStencilState;
				ID3D11ShaderResourceView* PSShaderResource;
				ID3D11SamplerState* PSSampler;
				ID3D11PixelShader* PS;
				ID3D11VertexShader* VS;
				UINT PSInstancesCount, VSInstancesCount;
				ID3D11ClassInstance* PSInstances[256], *VSInstances[256];
				D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology;
				ID3D11Buffer* IndexBuffer, *VertexBuffer, *VSConstantBuffer;
				UINT IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
				DXGI_FORMAT IndexBufferFormat;
				ID3D11InputLayout* InputLayout;

				D3D_StateBackup()
				{
					ScissorRectsCount = ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
					D3D.Context->RSGetScissorRects(&ScissorRectsCount, ScissorRects);
					D3D.Context->RSGetViewports(&ViewportsCount, Viewports);
					D3D.Context->RSGetState(&RS);
					D3D.Context->OMGetBlendState(&BlendState, BlendFactor, &SampleMask);
					D3D.Context->OMGetDepthStencilState(&DepthStencilState, &StencilRef);
					D3D.Context->PSGetShaderResources(0, 1, &PSShaderResource);
					D3D.Context->PSGetSamplers(0, 1, &PSSampler);
					PSInstancesCount = VSInstancesCount = 256;
					D3D.Context->PSGetShader(&PS, PSInstances, &PSInstancesCount);
					D3D.Context->VSGetShader(&VS, VSInstances, &VSInstancesCount);
					D3D.Context->VSGetConstantBuffers(0, 1, &VSConstantBuffer);
					D3D.Context->IAGetPrimitiveTopology(&PrimitiveTopology);
					D3D.Context->IAGetIndexBuffer(&IndexBuffer, &IndexBufferFormat, &IndexBufferOffset);
					D3D.Context->IAGetVertexBuffers(0, 1, &VertexBuffer, &VertexBufferStride, &VertexBufferOffset);
					D3D.Context->IAGetInputLayout(&InputLayout);
				}

				~D3D_StateBackup()
				{
					D3D.Context->RSSetScissorRects(ScissorRectsCount, ScissorRects);
					D3D.Context->RSSetViewports(ViewportsCount, Viewports);
					D3D.Context->RSSetState(RS); if (RS) RS->Release();
					D3D.Context->OMSetBlendState(BlendState, BlendFactor, SampleMask); if (BlendState) BlendState->Release();
					D3D.Context->OMSetDepthStencilState(DepthStencilState, StencilRef); if (DepthStencilState) DepthStencilState->Release();
					D3D.Context->PSSetShaderResources(0, 1, &PSShaderResource); if (PSShaderResource) PSShaderResource->Release();
					D3D.Context->PSSetSamplers(0, 1, &PSSampler); if (PSSampler) PSSampler->Release();
					D3D.Context->PSSetShader(PS, PSInstances, PSInstancesCount); if (PS) PS->Release();
					for (UINT i = 0; i < PSInstancesCount; i++) if (PSInstances[i]) PSInstances[i]->Release();
					D3D.Context->VSSetShader(VS, VSInstances, VSInstancesCount); if (VS) VS->Release();
					D3D.Context->VSSetConstantBuffers(0, 1, &VSConstantBuffer); if (VSConstantBuffer) VSConstantBuffer->Release();
					for (UINT i = 0; i < VSInstancesCount; i++) if (VSInstances[i]) VSInstances[i]->Release();
					D3D.Context->IASetPrimitiveTopology(PrimitiveTopology);
					D3D.Context->IASetIndexBuffer(IndexBuffer, IndexBufferFormat, IndexBufferOffset); if (IndexBuffer) IndexBuffer->Release();
					D3D.Context->IASetVertexBuffers(0, 1, &VertexBuffer, &VertexBufferStride, &VertexBufferOffset); if (VertexBuffer) VertexBuffer->Release();
					D3D.Context->IASetInputLayout(InputLayout); if (InputLayout) InputLayout->Release();
				}

			} stateBackup;

			D3D.SetViewport({ drawData->DisplaySize.x, drawData->DisplaySize.y });
			D3D.Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			Data.DeviceObjects->InputLayout->Bind();
			Data.DeviceObjects->VertexBuffer->Bind();
			Data.DeviceObjects->IndexBuffer->Bind();

			Data.DeviceObjects->DefaultShader.Bind();

			Data.DeviceObjects->MatrixCB.BindVertexShader();
			Data.DeviceObjects->DynamicCB.BindPixelShader();

			Data.DeviceObjects->FontSampler.Bind(0);

			Data.DeviceObjects->BlendState.Bind();
			Data.DeviceObjects->RasterizerState.Bind();
			Data.DeviceObjects->DepthStencilState.Bind();

			const ImTextureID invalidTextureID = ImTextureID(nullptr);
			ImTextureID lastBoundTextureID = invalidTextureID;

			int vertexOffset = 0, indexOffset = 0;
			for (int commandListIndex = 0; commandListIndex < drawData->CmdListsCount; commandListIndex++)
			{
				const ImDrawList* commandList = drawData->CmdLists[commandListIndex];
				for (int commandIndex = 0; commandIndex < commandList->CmdBuffer.Size; commandIndex++)
				{
					const ImDrawCmd* command = &commandList->CmdBuffer[commandIndex];

					if (command->UserCallback)
					{
						command->UserCallback(commandList, command);
					}
					else
					{
						const D3D11_RECT scissorRect =
						{
							static_cast<LONG>(command->ClipRect.x - drawData->DisplayPos.x),
							static_cast<LONG>(command->ClipRect.y - drawData->DisplayPos.y),
							static_cast<LONG>(command->ClipRect.z - drawData->DisplayPos.x),
							static_cast<LONG>(command->ClipRect.w - drawData->DisplayPos.y)
						};

						D3D.Context->RSSetScissorRects(1, &scissorRect);

						if (lastBoundTextureID != command->TextureId)
						{
							auto& texture = command->TextureId;
							lastBoundTextureID = command->TextureId;

							std::array<ID3D11ShaderResourceView*, 2> resourceViews = { nullptr, nullptr };
							resourceViews[texture.IsCubeMap] = texture;

							if (texture.IsCubeMap || texture.IsRGTC)
							{
								Data.DeviceObjects->DynamicCB.Data.RenderCubeMap = texture.IsCubeMap;
								Data.DeviceObjects->DynamicCB.Data.CubeMapFace = -1;
								Data.DeviceObjects->DynamicCB.Data.CubeMapUnwrapNet = true;
								Data.DeviceObjects->DynamicCB.Data.DecompressRGTC = texture.IsRGTC;
								Data.DeviceObjects->DynamicCB.UploadData();

								Data.DeviceObjects->CustomShader.Bind();
							}
							else
							{
								Data.DeviceObjects->DefaultShader.Bind();
							}

							D3D.Context->PSSetShaderResources(0, static_cast<UINT>(resourceViews.size()), resourceViews.data());
						}

						D3D.Context->DrawIndexed(command->ElemCount, indexOffset, vertexOffset);
					}

					indexOffset += command->ElemCount;
				}

				vertexOffset += commandList->VtxBuffer.Size;
			}
		}
	}

	namespace ComfyD3D11PlatformInterface
	{
		struct ViewportUserData
		{
			ComPtr<IDXGISwapChain> SwapChain = nullptr;
			ComPtr<ID3D11RenderTargetView> RenderTargetView = nullptr;
		};

		void PlatformCreateWindow(ImGuiViewport* viewport)
		{
			ViewportUserData* userData = new ViewportUserData();
			viewport->RendererUserData = userData;

			HWND windowHandle = static_cast<HWND>(viewport->PlatformHandle);
			assert(windowHandle != 0);

			DXGI_SWAP_CHAIN_DESC swapChainDescription = {};
			swapChainDescription.BufferDesc.Width = static_cast<UINT>(viewport->Size.x);
			swapChainDescription.BufferDesc.Height = static_cast<UINT>(viewport->Size.y);
			swapChainDescription.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapChainDescription.SampleDesc.Count = 1;
			swapChainDescription.SampleDesc.Quality = 0;
			swapChainDescription.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapChainDescription.BufferCount = 1;
			swapChainDescription.OutputWindow = windowHandle;
			swapChainDescription.Windowed = true;
			swapChainDescription.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapChainDescription.Flags = 0;

			assert(userData->SwapChain == nullptr && userData->RenderTargetView == nullptr);
			ComfyD3D11::Data.Factory->CreateSwapChain(D3D.Device, &swapChainDescription, &userData->SwapChain);

			if (userData->SwapChain)
			{
				ComPtr<ID3D11Texture2D> backBuffer = nullptr;
				userData->SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

				D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &userData->RenderTargetView);
			}
		}

		void PlatformDestroyWindow(ImGuiViewport* viewport)
		{
			auto userData = static_cast<ViewportUserData*>(viewport->RendererUserData);
			delete userData;

			viewport->RendererUserData = nullptr;
		}

		void PlatformSetWindowSize(ImGuiViewport* viewport, ImVec2 size)
		{
			auto userData = static_cast<ViewportUserData*>(viewport->RendererUserData);

			userData->RenderTargetView = nullptr;
			
			if (userData->SwapChain == nullptr)
				return;

			ComPtr<ID3D11Texture2D> backBuffer = nullptr;
			userData->SwapChain->ResizeBuffers(0, static_cast<UINT>(size.x), static_cast<UINT>(size.y), DXGI_FORMAT_UNKNOWN, 0);
			userData->SwapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

			D3D.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &userData->RenderTargetView);
		}

		void PlatformRenderWindow(ImGuiViewport* viewport, void*)
		{
			auto userData = static_cast<ViewportUserData*>(viewport->RendererUserData);

			std::array renderTargets = { userData->RenderTargetView.Get() };
			D3D.Context->OMSetRenderTargets(static_cast<UINT>(renderTargets.size()), renderTargets.data(), nullptr);

			if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
			{
				constexpr vec4 clearColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
				D3D.Context->ClearRenderTargetView(userData->RenderTargetView.Get(), glm::value_ptr(clearColor));
			}

			ComfyD3D11::RenderDrawData(viewport->DrawData);
		}

		void PlatformSwapBuffers(ImGuiViewport* viewport, void*)
		{
			auto userData = static_cast<ViewportUserData*>(viewport->RendererUserData);
			userData->SwapChain->Present(0, 0);
		}
	}
}
