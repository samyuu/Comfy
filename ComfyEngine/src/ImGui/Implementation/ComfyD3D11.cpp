#include "ComfyD3D11.h"
#include "Render/D3D11/D3D11.h"
#include "Render/D3D11/D3D11Buffer.h"
#include "Render/D3D11/D3D11Shader.h"
#include "Render/D3D11/D3D11State.h"
#include "Render/D3D11/D3D11Texture.h"
#include "Render/D3D11/Shader/Bytecode/ShaderBytecode.h"

using namespace Comfy::Graphics;
using namespace Comfy::Render;

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
			i32 DecompressRGTC;
			i32 RenderCubeMap;
			i32 CubeMapFace;
			i32 CubeMapUnwrapNet;
			i32 CubeMapMipLevel;
			i32 Padding[3];
		};

		struct ComfyD3D11DeviceObjects
		{
			std::unique_ptr<D3D11VertexBuffer> VertexBuffer = nullptr;
			std::unique_ptr<D3D11IndexBuffer> IndexBuffer = nullptr;

			D3D11ShaderPair DefaultShader = { GlobalD3D11, ImGuiDefault_VS(), ImGuiDefault_PS(), "ComfyD3D11::ImGuiDefault" };
			D3D11ShaderPair CustomShader = { GlobalD3D11, ImGuiDefault_VS(), ImGuiCustom_PS(), "ComfyD3D11::ImGuiCustom" };

			D3D11ConstantBufferTemplate<MatrixConstantData> MatrixCB = { GlobalD3D11, 0, D3D11_USAGE_DEFAULT, "ComfyD3D11::MatrixCB" };
			D3D11ConstantBufferTemplate<DynamicConstantData> DynamicCB = { GlobalD3D11, 0, D3D11_USAGE_DYNAMIC, "ComfyD3D11::DynamicCB" };

			std::unique_ptr<D3D11InputLayout> InputLayout = nullptr;

			D3D11TextureSampler FontSampler = { GlobalD3D11, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP };
			std::unique_ptr<D3D11Texture2DAndView> FontTexture = nullptr;

			// D3D11BlendState BlendState = { GlobalD3D11, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ZERO };
			D3D11BlendState BlendState = { GlobalD3D11, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_INV_SRC_ALPHA };

			D3D11RasterizerState RasterizerState = { GlobalD3D11, D3D11_FILL_SOLID, D3D11_CULL_NONE, true, "ComfyD3D11::RasterizerState" };
			D3D11DepthStencilState DepthStencilState = { GlobalD3D11, false, D3D11_DEPTH_WRITE_MASK_ALL, "ComfyD3D11::DepthStencilState" };

			ComfyD3D11DeviceObjects()
			{
				static constexpr D3D11InputElement elements[] =
				{
					{ "POSITION",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(ImDrawVert, pos)	},
					{ "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT,	offsetof(ImDrawVert, uv)	},
					{ "COLOR",		0, DXGI_FORMAT_R8G8B8A8_UNORM,	offsetof(ImDrawVert, col)	},
				};

				InputLayout = std::make_unique<D3D11InputLayout>(GlobalD3D11, elements, std::size(elements), DefaultShader.VS);
				D3D11_SetObjectDebugName(InputLayout->InputLayout.Get(), "ComfyD3D11::InputLayout");

				D3D11_SetObjectDebugName(FontSampler.SamplerState.Get(), "ComfyD3D11::FontSampler");
				D3D11_SetObjectDebugName(BlendState.BlendState.Get(), "ComfyD3D11::BlendState");

				CreateSetFontTexture();
			}

			~ComfyD3D11DeviceObjects()
			{
				GetIO().Fonts->SetTexID(nullptr);
			}

			void CreateSetFontTexture()
			{
				u8* rgbaPixels;
				ivec2 textureSize;
				GetIO().Fonts->GetTexDataAsRGBA32(&rgbaPixels, &textureSize.x, &textureSize.y);

				FontTexture = std::make_unique<D3D11Texture2DAndView>(GlobalD3D11, textureSize, reinterpret_cast<u32*>(rgbaPixels), D3D11_USAGE_IMMUTABLE);
				D3D11_SetObjectDebugName(FontTexture->Texture.Get(), "ComfyD3D11::FontTexture");
				D3D11_SetObjectDebugName(FontTexture->TextureView.Get(), "ComfyD3D11::FontTextureView");

				GetIO().Fonts->SetTexID(static_cast<ImTextureID>(*FontTexture));
			}
		};

		struct ComfyD3D11Data
		{
			ComPtr<IDXGIFactory> Factory = nullptr;
			std::unique_ptr<ComfyD3D11DeviceObjects> DeviceObjects = nullptr;
			i32 VertexBufferSize = 5000;
			i32 IndexBufferSize = 10000;
		} Data;

		bool Initialize()
		{
			auto& d3d11 = GlobalD3D11;

			ImGuiIO& io = GetIO();
			io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
			io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;
			io.BackendRendererName = "ComfyD3D11";

			ComPtr<IDXGIDevice> dxgiDevice = nullptr;
			ComPtr<IDXGIAdapter> dxgiAdapter = nullptr;

			if (d3d11.Device->QueryInterface(__uuidof(dxgiDevice), &dxgiDevice) != S_OK)
				return false;

			if (dxgiDevice->GetParent(__uuidof(dxgiAdapter), &dxgiAdapter) != S_OK)
				return false;

			if (dxgiAdapter->GetParent(__uuidof(Data.Factory), &Data.Factory) != S_OK)
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
			Data.DeviceObjects = std::make_unique<ComfyD3D11DeviceObjects>();
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
			auto& d3d11 = GlobalD3D11;

			if (GetIO().Fonts->TexID == nullptr)
				Data.DeviceObjects->CreateSetFontTexture();

			// NOTE: Avoid rendering when minimized
			if (drawData->DisplaySize.x <= 0.0f || drawData->DisplaySize.y <= 0.0f)
				return;

			D3D11_BeginDebugEvent("ImGui::RenderDrawData");

			if (Data.DeviceObjects->VertexBuffer == nullptr || Data.VertexBufferSize < drawData->TotalVtxCount)
			{
				D3D11_BeginDebugEvent("Create VBO");
				Data.VertexBufferSize = drawData->TotalVtxCount + 5000;
				Data.DeviceObjects->VertexBuffer = std::make_unique<D3D11VertexBuffer>(d3d11, Data.VertexBufferSize * sizeof(ImDrawVert), nullptr, sizeof(ImDrawVert), D3D11_USAGE_DYNAMIC);

				D3D11_SetObjectDebugName(Data.DeviceObjects->VertexBuffer->Buffer.Get(), "ComfyD3D11::VertexBuffer");
				D3D11_EndDebugEvent();
			}

			if (Data.DeviceObjects->IndexBuffer == nullptr || Data.IndexBufferSize < drawData->TotalIdxCount)
			{
				D3D11_BeginDebugEvent("Create IBO");
				Data.IndexBufferSize = drawData->TotalIdxCount + 10000;
				Data.DeviceObjects->IndexBuffer = std::make_unique<D3D11IndexBuffer>(d3d11, Data.IndexBufferSize * sizeof(ImDrawIdx), nullptr, IndexFormat::U16, D3D11_USAGE_DYNAMIC);

				D3D11_SetObjectDebugName(Data.DeviceObjects->IndexBuffer->Buffer.Get(), "ComfyD3D11::IndexBuffer");
				D3D11_EndDebugEvent();
			}

			// NOTE: Upload vertex / index data into a single contiguous GPU buffer
			D3D11_BeginDebugEvent("Update VBO / IBO");
			{
				D3D11_MAPPED_SUBRESOURCE mappedVertices, mappedIndices;
				d3d11.ImmediateContext->Map(Data.DeviceObjects->VertexBuffer->Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedVertices);
				d3d11.ImmediateContext->Map(Data.DeviceObjects->IndexBuffer->Buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedIndices);

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

				d3d11.ImmediateContext->Unmap(Data.DeviceObjects->VertexBuffer->Buffer.Get(), 0);
				d3d11.ImmediateContext->Unmap(Data.DeviceObjects->IndexBuffer->Buffer.Get(), 0);
			}
			D3D11_EndDebugEvent();

			{
				D3D11_BeginDebugEvent("Set Begin State");
				struct D3D11StateBackup
				{
					// TODO: Most of these are redundant
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

					D3D11StateBackup()
					{
						auto& d3d11 = GlobalD3D11;
						ScissorRectsCount = ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
						d3d11.ImmediateContext->RSGetScissorRects(&ScissorRectsCount, ScissorRects);
						d3d11.ImmediateContext->RSGetViewports(&ViewportsCount, Viewports);
						d3d11.ImmediateContext->RSGetState(&RS);
						d3d11.ImmediateContext->OMGetBlendState(&BlendState, BlendFactor, &SampleMask);
						d3d11.ImmediateContext->OMGetDepthStencilState(&DepthStencilState, &StencilRef);
						d3d11.ImmediateContext->PSGetShaderResources(0, 1, &PSShaderResource);
						d3d11.ImmediateContext->PSGetSamplers(0, 1, &PSSampler);
						PSInstancesCount = VSInstancesCount = 256;
						d3d11.ImmediateContext->PSGetShader(&PS, PSInstances, &PSInstancesCount);
						d3d11.ImmediateContext->VSGetShader(&VS, VSInstances, &VSInstancesCount);
						d3d11.ImmediateContext->VSGetConstantBuffers(0, 1, &VSConstantBuffer);
						d3d11.ImmediateContext->IAGetPrimitiveTopology(&PrimitiveTopology);
						d3d11.ImmediateContext->IAGetIndexBuffer(&IndexBuffer, &IndexBufferFormat, &IndexBufferOffset);
						d3d11.ImmediateContext->IAGetVertexBuffers(0, 1, &VertexBuffer, &VertexBufferStride, &VertexBufferOffset);
						d3d11.ImmediateContext->IAGetInputLayout(&InputLayout);
					}

					~D3D11StateBackup()
					{
						auto& d3d11 = GlobalD3D11;
						d3d11.ImmediateContext->RSSetScissorRects(ScissorRectsCount, ScissorRects);
						d3d11.ImmediateContext->RSSetViewports(ViewportsCount, Viewports);
						d3d11.ImmediateContext->RSSetState(RS); if (RS) RS->Release();
						d3d11.ImmediateContext->OMSetBlendState(BlendState, BlendFactor, SampleMask); if (BlendState) BlendState->Release();
						d3d11.ImmediateContext->OMSetDepthStencilState(DepthStencilState, StencilRef); if (DepthStencilState) DepthStencilState->Release();
						d3d11.ImmediateContext->PSSetShaderResources(0, 1, &PSShaderResource); if (PSShaderResource) PSShaderResource->Release();
						d3d11.ImmediateContext->PSSetSamplers(0, 1, &PSSampler); if (PSSampler) PSSampler->Release();
						d3d11.ImmediateContext->PSSetShader(PS, PSInstances, PSInstancesCount); if (PS) PS->Release();
						for (UINT i = 0; i < PSInstancesCount; i++) if (PSInstances[i]) PSInstances[i]->Release();
						d3d11.ImmediateContext->VSSetShader(VS, VSInstances, VSInstancesCount); if (VS) VS->Release();
						d3d11.ImmediateContext->VSSetConstantBuffers(0, 1, &VSConstantBuffer); if (VSConstantBuffer) VSConstantBuffer->Release();
						for (UINT i = 0; i < VSInstancesCount; i++) if (VSInstances[i]) VSInstances[i]->Release();
						d3d11.ImmediateContext->IASetPrimitiveTopology(PrimitiveTopology);
						d3d11.ImmediateContext->IASetIndexBuffer(IndexBuffer, IndexBufferFormat, IndexBufferOffset); if (IndexBuffer) IndexBuffer->Release();
						d3d11.ImmediateContext->IASetVertexBuffers(0, 1, &VertexBuffer, &VertexBufferStride, &VertexBufferOffset); if (VertexBuffer) VertexBuffer->Release();
						d3d11.ImmediateContext->IASetInputLayout(InputLayout); if (InputLayout) InputLayout->Release();
					}
				} stateBackup = {};

				// NOTE: Setup orthographic projection matrix
				{
					const f32 left = drawData->DisplayPos.x;
					const f32 right = drawData->DisplayPos.x + drawData->DisplaySize.x;
					const f32 top = drawData->DisplayPos.y;
					const f32 bottom = drawData->DisplayPos.y + drawData->DisplaySize.y;

					Data.DeviceObjects->MatrixCB.Data.ModelViewProjection = glm::transpose(glm::ortho(left, right, bottom, top, 0.0f, 1.0f));
					Data.DeviceObjects->MatrixCB.UploadData(d3d11);
				}

				d3d11.SetViewport({ drawData->DisplaySize.x, drawData->DisplaySize.y });
				d3d11.ImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				Data.DeviceObjects->InputLayout->Bind(d3d11);
				Data.DeviceObjects->VertexBuffer->Bind(d3d11);
				Data.DeviceObjects->IndexBuffer->Bind(d3d11);

				Data.DeviceObjects->MatrixCB.BindVertexShader(d3d11);
				Data.DeviceObjects->DynamicCB.BindPixelShader(d3d11);

				Data.DeviceObjects->FontSampler.Bind(d3d11, 0);

				Data.DeviceObjects->BlendState.Bind(d3d11);
				Data.DeviceObjects->RasterizerState.Bind(d3d11);
				Data.DeviceObjects->DepthStencilState.Bind(d3d11);
				D3D11_EndDebugEvent();

				struct D3D11StateCache
				{
					const D3D11ShaderPair* Shader = nullptr;

					const vec4 InvalidScissorRect = vec4(0.0f);
					vec4 ScissorRect = InvalidScissorRect;

					const ImTextureID InvalidTextureID = ImTextureID(nullptr);
					ImTextureID TextureID = InvalidTextureID;
				} cache = {};

				int vertexOffset = 0, indexOffset = 0;
				for (int commandListIndex = 0; commandListIndex < drawData->CmdListsCount; commandListIndex++)
				{
					D3D11_BeginDebugEvent("Command List");

					const ImDrawList& commandList = *drawData->CmdLists[commandListIndex];
					for (int commandIndex = 0; commandIndex < commandList.CmdBuffer.Size; commandIndex++)
					{
						const ImDrawCmd& command = commandList.CmdBuffer[commandIndex];

						if (command.UserCallback)
						{
							D3D11_BeginDebugEvent("User Callback");
							command.UserCallback(&commandList, &command);
							D3D11_EndDebugEvent();
						}
						else
						{
							const vec4 commandScissorRect = vec4(command.ClipRect) - vec4(vec2(drawData->DisplayPos), vec2(drawData->DisplayPos));
							if (commandScissorRect != cache.ScissorRect)
							{
								d3d11.SetScissorRect(commandScissorRect);
								cache.ScissorRect = commandScissorRect;
							}

							const D3D11ShaderPair* commandShader = &Data.DeviceObjects->DefaultShader;
							const ImTextureID& commandTexture = command.TextureId;

							if (commandTexture != cache.TextureID)
							{
								cache.TextureID = commandTexture;

								if (commandTexture.Data.IsCubeMap || commandTexture.Data.DecompressRGTC)
								{
									D3D11_SetDebugMarker("Set Custom Shader");
									commandShader = &Data.DeviceObjects->CustomShader;

									Data.DeviceObjects->DynamicCB.Data.DecompressRGTC = !commandTexture.Data.IsCubeMap && commandTexture.Data.DecompressRGTC;
									Data.DeviceObjects->DynamicCB.Data.RenderCubeMap = commandTexture.Data.IsCubeMap;
									Data.DeviceObjects->DynamicCB.Data.CubeMapFace = -1;
									Data.DeviceObjects->DynamicCB.Data.CubeMapUnwrapNet = true;
									Data.DeviceObjects->DynamicCB.Data.CubeMapMipLevel = commandTexture.Data.CubeMapMipLevel;
									Data.DeviceObjects->DynamicCB.UploadData(d3d11);

									std::array<ID3D11ShaderResourceView*, 2> resourceViews = { nullptr, nullptr };
									resourceViews[commandTexture.Data.IsCubeMap] = commandTexture.GetResourceView();

									d3d11.ImmediateContext->PSSetShaderResources(0, static_cast<UINT>(resourceViews.size()), resourceViews.data());
								}
								else
								{
									std::array<ID3D11ShaderResourceView*, 1> resourceViews = { commandTexture.GetResourceView() };
									d3d11.ImmediateContext->PSSetShaderResources(0, static_cast<UINT>(resourceViews.size()), resourceViews.data());
								}
							}

							if (commandShader != cache.Shader)
							{
								commandShader->Bind(d3d11);
								cache.Shader = commandShader;
							}

							d3d11.ImmediateContext->DrawIndexed(command.ElemCount, indexOffset, vertexOffset);
						}

						indexOffset += command.ElemCount;
					}

					vertexOffset += commandList.VtxBuffer.Size;
					D3D11_EndDebugEvent();
				}
			}

			D3D11_EndDebugEvent();
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

			HWND windowHandle = static_cast<HWND>(viewport->PlatformHandleRaw ? viewport->PlatformHandleRaw : viewport->PlatformHandle);
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

			auto& d3d11 = GlobalD3D11;

			assert(userData->SwapChain == nullptr && userData->RenderTargetView == nullptr);
			ComfyD3D11::Data.Factory->CreateSwapChain(d3d11.Device.Get(), &swapChainDescription, &userData->SwapChain);

			if (userData->SwapChain)
			{
				ComPtr<ID3D11Texture2D> backBuffer = nullptr;
				userData->SwapChain->GetBuffer(0, __uuidof(backBuffer), &backBuffer);

				d3d11.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &userData->RenderTargetView);
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

			auto& d3d11 = GlobalD3D11;

			ComPtr<ID3D11Texture2D> backBuffer = nullptr;
			userData->SwapChain->ResizeBuffers(0, static_cast<UINT>(size.x), static_cast<UINT>(size.y), DXGI_FORMAT_UNKNOWN, 0);
			userData->SwapChain->GetBuffer(0, __uuidof(backBuffer), &backBuffer);

			d3d11.Device->CreateRenderTargetView(backBuffer.Get(), nullptr, &userData->RenderTargetView);
		}

		void PlatformRenderWindow(ImGuiViewport* viewport, void*)
		{
			D3D11_BeginDebugEvent("ImGui::PlatformRenderWindow");
			auto userData = static_cast<ViewportUserData*>(viewport->RendererUserData);

			auto& d3d11 = GlobalD3D11;

			std::array renderTargets = { userData->RenderTargetView.Get() };
			d3d11.ImmediateContext->OMSetRenderTargets(static_cast<UINT>(renderTargets.size()), renderTargets.data(), nullptr);

			if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
			{
				constexpr vec4 clearColor = vec4(0.0f, 0.0f, 0.0f, 1.0f);
				d3d11.ImmediateContext->ClearRenderTargetView(userData->RenderTargetView.Get(), glm::value_ptr(clearColor));
			}

			ComfyD3D11::RenderDrawData(viewport->DrawData);
			D3D11_EndDebugEvent();
		}

		void PlatformSwapBuffers(ImGuiViewport* viewport, void*)
		{
			auto userData = static_cast<ViewportUserData*>(viewport->RendererUserData);
			userData->SwapChain->Present(0, 0);
		}
	}
}
