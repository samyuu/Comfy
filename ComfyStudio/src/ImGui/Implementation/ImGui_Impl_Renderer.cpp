// dear imgui: Renderer for DirectX11
// This needs to be used along with a Platform Binding (e.g. Win32)

// Implemented features:
//  [X] Renderer: User texture binding. Use 'ID3D11ShaderResourceView*' as ImTextureID. Read the FAQ about ImTextureID in imgui.cpp.
//  [X] Renderer: Multi-viewport support. Enable with 'io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable'.

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read examples/README.txt and read the documentation at the top of imgui.cpp
// https://github.com/ocornut/imgui

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2018-XX-XX: Platform: Added support for multiple windows via the ImGuiPlatformIO interface.
//  2018-12-03: Misc: Added #pragma comment statement to automatically link with d3dcompiler.lib when using D3DCompile().
//  2018-11-30: Misc: Setting up io.BackendRendererName so it can be displayed in the About Window.
//  2018-08-01: DirectX11: Querying for IDXGIFactory instead of IDXGIFactory1 to increase compatibility.
//  2018-07-13: DirectX11: Fixed unreleased resources in Init and Shutdown functions.
//  2018-06-08: Misc: Extracted imgui_impl_dx11.cpp/.h away from the old combined DX11+Win32 example.
//  2018-06-08: DirectX11: Use draw_data->DisplayPos and draw_data->DisplaySize to setup projection matrix and clipping rectangle.
//  2018-02-16: Misc: Obsoleted the io.RenderDrawListsFn callback and exposed ImGui_ImplDX11_RenderDrawData() in the .h file so you can call it yourself.
//  2018-02-06: Misc: Removed call to ImGui::Shutdown() which is not available from 1.60 WIP, user needs to call CreateContext/DestroyContext themselves.
//  2016-05-07: DirectX11: Disabling depth-write.

#include "ImGui/Core/imgui.h"
#include "ImGui/Implementation/ImGui_Impl_Renderer.h"

#include "Graphics/Direct3D/Direct3D.h"
#include "Graphics/Direct3D/ShaderBytecode/ShaderBytecode.h"

using namespace Graphics;

// DirectX data
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGIFactory*            g_pFactory = nullptr;
static ID3D11Buffer*            g_pVB = nullptr;
static ID3D11Buffer*            g_pIB = nullptr;
static ID3D11VertexShader*      g_pVertexShader = nullptr;
static ID3D11InputLayout*       g_pInputLayout = nullptr;
static ID3D11Buffer*            g_pVertexConstantBuffer = nullptr;
static ID3D11PixelShader*       g_pPixelShader = nullptr;
static ID3D11SamplerState*      g_pFontSampler = nullptr;
static ID3D11ShaderResourceView*g_pFontTextureView = nullptr;
static ID3D11RasterizerState*   g_pRasterizerState = nullptr;
static ID3D11BlendState*        g_pBlendState = nullptr;
static ID3D11DepthStencilState* g_pDepthStencilState = nullptr;
static int                      g_VertexBufferSize = 5000, g_IndexBufferSize = 10000;

struct VERTEX_CONSTANT_BUFFER
{
	mat4 MVP;
};

// Forward Declarations
static void ImGui_ImplDX11_InitPlatformInterface();
static void ImGui_ImplDX11_ShutdownPlatformInterface();

// Render function
// (this used to be set in io.RenderDrawListsFn and called by ImGui::Render(), but you can now call this directly from your main loop)
void ImGui_ImplDX11_RenderDrawData(ImDrawData* draw_data)
{
	// Avoid rendering when minimized
	if (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f)
		return;

	ID3D11DeviceContext* ctx = g_pd3dDeviceContext;

	// Create and grow vertex/index buffers if needed
	if (!g_pVB || g_VertexBufferSize < draw_data->TotalVtxCount)
	{
		if (g_pVB) { g_pVB->Release(); g_pVB = nullptr; }

		g_VertexBufferSize = draw_data->TotalVtxCount + 5000;

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = g_VertexBufferSize * sizeof(ImDrawVert);
		desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;

		if (g_pd3dDevice->CreateBuffer(&desc, nullptr, &g_pVB) < 0)
			return;

		D3D_SetObjectDebugName(g_pVB, "ImGui::VertexBuffer");
	}

	if (!g_pIB || g_IndexBufferSize < draw_data->TotalIdxCount)
	{
		if (g_pIB) { g_pIB->Release(); g_pIB = nullptr; }

		g_IndexBufferSize = draw_data->TotalIdxCount + 10000;

		D3D11_BUFFER_DESC desc = {};
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = g_IndexBufferSize * sizeof(ImDrawIdx);
		desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		if (g_pd3dDevice->CreateBuffer(&desc, nullptr, &g_pIB) < 0)
			return;

		D3D_SetObjectDebugName(g_pIB, "ImGui::IndexBuffer");
	}

	// Upload vertex/index data into a single contiguous GPU buffer
	D3D11_MAPPED_SUBRESOURCE vtx_resource, idx_resource;
	if (ctx->Map(g_pVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &vtx_resource) != S_OK)
		return;

	if (ctx->Map(g_pIB, 0, D3D11_MAP_WRITE_DISCARD, 0, &idx_resource) != S_OK)
		return;

	ImDrawVert* vtx_dst = static_cast<ImDrawVert*>(vtx_resource.pData);
	ImDrawIdx* idx_dst = static_cast<ImDrawIdx*>(idx_resource.pData);
	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		memcpy(vtx_dst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
		memcpy(idx_dst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
		vtx_dst += cmd_list->VtxBuffer.Size;
		idx_dst += cmd_list->IdxBuffer.Size;
	}
	ctx->Unmap(g_pVB, 0);
	ctx->Unmap(g_pIB, 0);

	// Setup orthographic projection matrix into our constant buffer
	// Our visible imgui space lies from draw_data->DisplayPos (top left) to draw_data->DisplayPos+data_data->DisplaySize (bottom right). DisplayMin is (0,0) for single viewport apps.
	{
		D3D11_MAPPED_SUBRESOURCE mapped_resource;
		if (ctx->Map(g_pVertexConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_resource) != S_OK)
			return;

		VERTEX_CONSTANT_BUFFER* constant_buffer = static_cast<VERTEX_CONSTANT_BUFFER*>(mapped_resource.pData);
		float left = draw_data->DisplayPos.x;
		float right = draw_data->DisplayPos.x + draw_data->DisplaySize.x;
		float top = draw_data->DisplayPos.y;
		float bottom = draw_data->DisplayPos.y + draw_data->DisplaySize.y;
		constant_buffer->MVP = glm::transpose(glm::ortho(left, right, bottom, top, 0.0f, 1.0f));

		ctx->Unmap(g_pVertexConstantBuffer, 0);
	}

	// Backup DX state that will be modified to restore it afterwards (unfortunately this is very ugly looking and verbose. Close your eyes!)
	struct BACKUP_DX11_STATE
	{
		UINT                        ScissorRectsCount, ViewportsCount;
		D3D11_RECT                  ScissorRects[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		D3D11_VIEWPORT              Viewports[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
		ID3D11RasterizerState*      RS;
		ID3D11BlendState*           BlendState;
		FLOAT                       BlendFactor[4];
		UINT                        SampleMask;
		UINT                        StencilRef;
		ID3D11DepthStencilState*    DepthStencilState;
		ID3D11ShaderResourceView*   PSShaderResource;
		ID3D11SamplerState*         PSSampler;
		ID3D11PixelShader*          PS;
		ID3D11VertexShader*         VS;
		UINT                        PSInstancesCount, VSInstancesCount;
		ID3D11ClassInstance*        PSInstances[256], *VSInstances[256];   // 256 is max according to PSSetShader documentation
		D3D11_PRIMITIVE_TOPOLOGY    PrimitiveTopology;
		ID3D11Buffer*               IndexBuffer, *VertexBuffer, *VSConstantBuffer;
		UINT                        IndexBufferOffset, VertexBufferStride, VertexBufferOffset;
		DXGI_FORMAT                 IndexBufferFormat;
		ID3D11InputLayout*          InputLayout;
	};

	BACKUP_DX11_STATE old;
	old.ScissorRectsCount = old.ViewportsCount = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
	ctx->RSGetScissorRects(&old.ScissorRectsCount, old.ScissorRects);
	ctx->RSGetViewports(&old.ViewportsCount, old.Viewports);
	ctx->RSGetState(&old.RS);
	ctx->OMGetBlendState(&old.BlendState, old.BlendFactor, &old.SampleMask);
	ctx->OMGetDepthStencilState(&old.DepthStencilState, &old.StencilRef);
	ctx->PSGetShaderResources(0, 1, &old.PSShaderResource);
	ctx->PSGetSamplers(0, 1, &old.PSSampler);
	old.PSInstancesCount = old.VSInstancesCount = 256;
	ctx->PSGetShader(&old.PS, old.PSInstances, &old.PSInstancesCount);
	ctx->VSGetShader(&old.VS, old.VSInstances, &old.VSInstancesCount);
	ctx->VSGetConstantBuffers(0, 1, &old.VSConstantBuffer);
	ctx->IAGetPrimitiveTopology(&old.PrimitiveTopology);
	ctx->IAGetIndexBuffer(&old.IndexBuffer, &old.IndexBufferFormat, &old.IndexBufferOffset);
	ctx->IAGetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset);
	ctx->IAGetInputLayout(&old.InputLayout);

	// Setup viewport
	D3D11_VIEWPORT vp = {};
	vp.Width = draw_data->DisplaySize.x;
	vp.Height = draw_data->DisplaySize.y;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = vp.TopLeftY = 0;
	ctx->RSSetViewports(1, &vp);

	// Bind shader and vertex buffers
	unsigned int stride = sizeof(ImDrawVert);
	unsigned int offset = 0;
	ctx->IASetInputLayout(g_pInputLayout);
	ctx->IASetVertexBuffers(0, 1, &g_pVB, &stride, &offset);
	ctx->IASetIndexBuffer(g_pIB, sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT, 0);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ctx->VSSetShader(g_pVertexShader, nullptr, 0);
	ctx->VSSetConstantBuffers(0, 1, &g_pVertexConstantBuffer);
	ctx->PSSetShader(g_pPixelShader, nullptr, 0);
	ctx->PSSetSamplers(0, 1, &g_pFontSampler);

	// Setup render state
	const float blend_factor[4] = { 0.f, 0.f, 0.f, 0.f };
	ctx->OMSetBlendState(g_pBlendState, blend_factor, 0xffffffff);
	ctx->OMSetDepthStencilState(g_pDepthStencilState, 0);
	ctx->RSSetState(g_pRasterizerState);

	// Render command lists
	int vtx_offset = 0;
	int idx_offset = 0;
	ImVec2 clip_off = draw_data->DisplayPos;

	const ImVec4 invalid_clip_rect = ImVec4(-1.0f, -1.0f, -1.0f, -1.0f);
	ImVec4 last_clip_rect = invalid_clip_rect;

	const ImTextureID invalid_texture_id = (ImTextureID)(-1);
	ImTextureID last_texture_id = invalid_texture_id;

	for (int n = 0; n < draw_data->CmdListsCount; n++)
	{
		const ImDrawList* cmd_list = draw_data->CmdLists[n];
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
			if (pcmd->UserCallback)
			{
				// User callback (registered via ImDrawList::AddCallback)
				pcmd->UserCallback(cmd_list, pcmd);
			}
			else
			{
				if (std::memcmp(&last_clip_rect, &pcmd->ClipRect, sizeof(ImVec4)) != 0)
				{
					// Apply scissor/clipping rectangle
					const D3D11_RECT r =
					{
						static_cast<LONG>(pcmd->ClipRect.x - clip_off.x),
						static_cast<LONG>(pcmd->ClipRect.y - clip_off.y),
						static_cast<LONG>(pcmd->ClipRect.z - clip_off.x),
						static_cast<LONG>(pcmd->ClipRect.w - clip_off.y)
					};
					ctx->RSSetScissorRects(1, &r);

					last_clip_rect = pcmd->ClipRect;
				}

				if (last_texture_id != pcmd->TextureId)
				{
					// Bind texture, Draw
					ID3D11ShaderResourceView* texture_srv = static_cast<ID3D11ShaderResourceView*>(pcmd->TextureId);
					ctx->PSSetShaderResources(0, 1, &texture_srv);

					last_texture_id = pcmd->TextureId;
				}

				ctx->DrawIndexed(pcmd->ElemCount, idx_offset, vtx_offset);
			}
			idx_offset += pcmd->ElemCount;
		}
		vtx_offset += cmd_list->VtxBuffer.Size;
	}

	// Restore modified DX state
	ctx->RSSetScissorRects(old.ScissorRectsCount, old.ScissorRects);
	ctx->RSSetViewports(old.ViewportsCount, old.Viewports);
	ctx->RSSetState(old.RS); if (old.RS) old.RS->Release();
	ctx->OMSetBlendState(old.BlendState, old.BlendFactor, old.SampleMask); if (old.BlendState) old.BlendState->Release();
	ctx->OMSetDepthStencilState(old.DepthStencilState, old.StencilRef); if (old.DepthStencilState) old.DepthStencilState->Release();
	ctx->PSSetShaderResources(0, 1, &old.PSShaderResource); if (old.PSShaderResource) old.PSShaderResource->Release();
	ctx->PSSetSamplers(0, 1, &old.PSSampler); if (old.PSSampler) old.PSSampler->Release();
	ctx->PSSetShader(old.PS, old.PSInstances, old.PSInstancesCount); if (old.PS) old.PS->Release();
	for (UINT i = 0; i < old.PSInstancesCount; i++) if (old.PSInstances[i]) old.PSInstances[i]->Release();
	ctx->VSSetShader(old.VS, old.VSInstances, old.VSInstancesCount); if (old.VS) old.VS->Release();
	ctx->VSSetConstantBuffers(0, 1, &old.VSConstantBuffer); if (old.VSConstantBuffer) old.VSConstantBuffer->Release();
	for (UINT i = 0; i < old.VSInstancesCount; i++) if (old.VSInstances[i]) old.VSInstances[i]->Release();
	ctx->IASetPrimitiveTopology(old.PrimitiveTopology);
	ctx->IASetIndexBuffer(old.IndexBuffer, old.IndexBufferFormat, old.IndexBufferOffset); if (old.IndexBuffer) old.IndexBuffer->Release();
	ctx->IASetVertexBuffers(0, 1, &old.VertexBuffer, &old.VertexBufferStride, &old.VertexBufferOffset); if (old.VertexBuffer) old.VertexBuffer->Release();
	ctx->IASetInputLayout(old.InputLayout); if (old.InputLayout) old.InputLayout->Release();
}

static void ImGui_ImplDX11_CreateFontsTexture()
{
	// Build texture atlas
	ImGuiIO& io = ImGui::GetIO();
	unsigned char* pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	// Upload texture to graphics system
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;

		ID3D11Texture2D* pTexture = nullptr;
		D3D11_SUBRESOURCE_DATA subResource;
		subResource.pSysMem = pixels;
		subResource.SysMemPitch = desc.Width * 4;
		subResource.SysMemSlicePitch = 0;
		g_pd3dDevice->CreateTexture2D(&desc, &subResource, &pTexture);
		D3D_SetObjectDebugName(pTexture, "ImGui::FontTexture");

		// Create texture view
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = desc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		g_pd3dDevice->CreateShaderResourceView(pTexture, &srvDesc, &g_pFontTextureView);
		D3D_SetObjectDebugName(g_pFontTextureView, "ImGui::FontTextureView");

		pTexture->Release();
	}

	// Store our identifier
	io.Fonts->TexID = static_cast<ImTextureID>(g_pFontTextureView);

	// Create texture sampler
	{
		D3D11_SAMPLER_DESC desc = {};
		desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		desc.MipLODBias = 0.f;
		desc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		desc.MinLOD = 0.f;
		desc.MaxLOD = 0.f;
		g_pd3dDevice->CreateSamplerState(&desc, &g_pFontSampler);
		D3D_SetObjectDebugName(g_pFontSampler, "ImGui::FontSampler");
	}
}

bool ImGui_ImplDX11_CreateDeviceObjects()
{
	if (!g_pd3dDevice)
		return false;

	if (g_pFontSampler)
		ImGui_ImplDX11_InvalidateDeviceObjects();

	// By using D3DCompile() from <d3dcompiler.h> / d3dcompiler.lib, we introduce a dependency to a given version of d3dcompiler_XX.dll (see D3DCOMPILER_DLL_A)
	// If you would like to use this DX11 sample code but remove this dependency you can:
	//  1) compile once, save the compiled shader blobs into a file or source code and pass them to CreateVertexShader()/CreatePixelShader() [preferred solution]
	//  2) use code to detect any version of the DLL and grab a pointer to D3DCompile from the DLL.
	// See https://github.com/ocornut/imgui/pull/638 for sources and details.

	// Create the vertex shader
	{
		auto[vertexShader, vertexShaderSize] = Graphics::ImGuiDefault_VS();

		if (g_pd3dDevice->CreateVertexShader(vertexShader, vertexShaderSize, nullptr, &g_pVertexShader) != S_OK)
			return false;

		D3D_SetObjectDebugName(g_pVertexShader, "ImGui::VertexShader");

		// Create the input layout
		D3D11_INPUT_ELEMENT_DESC local_layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, IM_OFFSETOF(ImDrawVert, pos), D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, IM_OFFSETOF(ImDrawVert, uv),  D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, IM_OFFSETOF(ImDrawVert, col), D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		if (g_pd3dDevice->CreateInputLayout(local_layout, 3, vertexShader, vertexShaderSize, &g_pInputLayout) != S_OK)
			return false;

		D3D_SetObjectDebugName(g_pInputLayout, "ImGui::InputLayout");

		// Create the constant buffer
		{
			D3D11_BUFFER_DESC desc = {};
			desc.ByteWidth = sizeof(VERTEX_CONSTANT_BUFFER);
			desc.Usage = D3D11_USAGE_DYNAMIC;
			desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
			desc.MiscFlags = 0;
			g_pd3dDevice->CreateBuffer(&desc, nullptr, &g_pVertexConstantBuffer);
			D3D_SetObjectDebugName(g_pFontSampler, "ImGui::VertexConstantBuffer");
		}
	}

	// Create the pixel shader
	{
		auto[pixelShader, pixelShaderSize] = Graphics::ImGuiDefault_PS();

		if (g_pd3dDevice->CreatePixelShader(pixelShader, pixelShaderSize, nullptr, &g_pPixelShader) != S_OK)
			return false;

		D3D_SetObjectDebugName(g_pPixelShader, "ImGui::PixelShader");
	}

	// Create the blending setup
	{
		D3D11_BLEND_DESC desc = {};
		desc.AlphaToCoverageEnable = false;
		desc.RenderTarget[0].BlendEnable = true;
		desc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		desc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
		desc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		desc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
		g_pd3dDevice->CreateBlendState(&desc, &g_pBlendState);
		D3D_SetObjectDebugName(g_pBlendState, "ImGui::BlendState");
	}

	// Create the rasterizer state
	{
		D3D11_RASTERIZER_DESC desc = {};
		desc.FillMode = D3D11_FILL_SOLID;
		desc.CullMode = D3D11_CULL_NONE;
		desc.ScissorEnable = true;
		desc.DepthClipEnable = true;
		g_pd3dDevice->CreateRasterizerState(&desc, &g_pRasterizerState);
		D3D_SetObjectDebugName(g_pRasterizerState, "ImGui::RasterizerState");
	}

	// Create depth-stencil State
	{
		D3D11_DEPTH_STENCIL_DESC desc = {};
		desc.DepthEnable = false;
		desc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		desc.DepthFunc = D3D11_COMPARISON_ALWAYS;
		desc.StencilEnable = false;
		desc.FrontFace.StencilFailOp = desc.FrontFace.StencilDepthFailOp = desc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		desc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		desc.BackFace = desc.FrontFace;
		g_pd3dDevice->CreateDepthStencilState(&desc, &g_pDepthStencilState);
		D3D_SetObjectDebugName(g_pDepthStencilState, "ImGui::DepthStencilState");
	}

	ImGui_ImplDX11_CreateFontsTexture();

	return true;
}

void ImGui_ImplDX11_InvalidateDeviceObjects()
{
	if (!g_pd3dDevice)
		return;

	if (g_pFontSampler) { g_pFontSampler->Release(); g_pFontSampler = nullptr; }
	if (g_pFontTextureView) { g_pFontTextureView->Release(); g_pFontTextureView = nullptr; ImGui::GetIO().Fonts->TexID = nullptr; } // We copied g_pFontTextureView to io.Fonts->TexID so let's clear that as well.
	if (g_pIB) { g_pIB->Release(); g_pIB = nullptr; }
	if (g_pVB) { g_pVB->Release(); g_pVB = nullptr; }

	if (g_pBlendState) { g_pBlendState->Release(); g_pBlendState = nullptr; }
	if (g_pDepthStencilState) { g_pDepthStencilState->Release(); g_pDepthStencilState = nullptr; }
	if (g_pRasterizerState) { g_pRasterizerState->Release(); g_pRasterizerState = nullptr; }
	if (g_pPixelShader) { g_pPixelShader->Release(); g_pPixelShader = nullptr; }
	if (g_pVertexConstantBuffer) { g_pVertexConstantBuffer->Release(); g_pVertexConstantBuffer = nullptr; }
	if (g_pInputLayout) { g_pInputLayout->Release(); g_pInputLayout = nullptr; }
	if (g_pVertexShader) { g_pVertexShader->Release(); g_pVertexShader = nullptr; }
}

bool ImGui_ImplDX11_Init()
{
	ID3D11Device* device = Graphics::D3D.Device;
	ID3D11DeviceContext* device_context = Graphics::D3D.Context;

	// Setup back-end capabilities flags
	ImGuiIO& io = ImGui::GetIO();
	io.BackendFlags |= ImGuiBackendFlags_RendererHasViewports;    // We can create multi-viewports on the Renderer side (optional)
	io.BackendRendererName = "ComfyD3D11";

	// Get factory from device
	IDXGIDevice* pDXGIDevice = nullptr;
	IDXGIAdapter* pDXGIAdapter = nullptr;
	IDXGIFactory* pFactory = nullptr;

	if (device->QueryInterface(IID_PPV_ARGS(&pDXGIDevice)) == S_OK &&
		pDXGIDevice->GetParent(IID_PPV_ARGS(&pDXGIAdapter)) == S_OK &&
		pDXGIAdapter->GetParent(IID_PPV_ARGS(&pFactory)) == S_OK)
	{
		g_pd3dDevice = device;
		g_pd3dDeviceContext = device_context;
		g_pFactory = pFactory;
	}

	if (pDXGIDevice)
		pDXGIDevice->Release();

	if (pDXGIAdapter)
		pDXGIAdapter->Release();

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		ImGui_ImplDX11_InitPlatformInterface();

	return true;
}

void ImGui_ImplDX11_Shutdown()
{
	ImGui_ImplDX11_ShutdownPlatformInterface();
	ImGui_ImplDX11_InvalidateDeviceObjects();

	if (g_pFactory)
	{
		g_pFactory->Release();
		g_pFactory = nullptr;
	}

	g_pd3dDevice = nullptr;
	g_pd3dDeviceContext = nullptr;
}

void ImGui_ImplDX11_NewFrame()
{
	if (!g_pFontSampler)
		ImGui_ImplDX11_CreateDeviceObjects();
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear imgui or creating a new binding for dear imgui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct ImGuiViewportDataDx11
{
	IDXGISwapChain*             SwapChain;
	ID3D11RenderTargetView*     RTView;

	ImGuiViewportDataDx11() { SwapChain = nullptr; RTView = nullptr; }
	~ImGuiViewportDataDx11() { IM_ASSERT(SwapChain == nullptr && RTView == nullptr); }
};

static void ImGui_ImplDX11_CreateWindow(ImGuiViewport* viewport)
{
	ImGuiViewportDataDx11* data = IM_NEW(ImGuiViewportDataDx11)();
	viewport->RendererUserData = data;

	HWND hwnd = (HWND)viewport->PlatformHandle;
	IM_ASSERT(hwnd != 0);

	// Create swap chain
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = static_cast<UINT>(viewport->Size.x);
	sd.BufferDesc.Height = static_cast<UINT>(viewport->Size.y);
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hwnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	IM_ASSERT(data->SwapChain == nullptr && data->RTView == nullptr);
	g_pFactory->CreateSwapChain(g_pd3dDevice, &sd, &data->SwapChain);

	// Create the render target
	if (data->SwapChain)
	{
		ID3D11Texture2D* pBackBuffer;
		data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &data->RTView);
		pBackBuffer->Release();
	}
}

static void ImGui_ImplDX11_DestroyWindow(ImGuiViewport* viewport)
{
	// The main viewport (owned by the application) will always have RendererUserData == NULL since we didn't create the data for it.
	ImGuiViewportDataDx11* data = static_cast<ImGuiViewportDataDx11*>(viewport->RendererUserData);
	{
		if (data->SwapChain)
			data->SwapChain->Release();
		data->SwapChain = nullptr;
		if (data->RTView)
			data->RTView->Release();
		data->RTView = nullptr;
		IM_DELETE(data);
	}
	viewport->RendererUserData = nullptr;
}

static void ImGui_ImplDX11_SetWindowSize(ImGuiViewport* viewport, ImVec2 size)
{
	ImGuiViewportDataDx11* data = static_cast<ImGuiViewportDataDx11*>(viewport->RendererUserData);
	if (data->RTView)
	{
		data->RTView->Release();
		data->RTView = nullptr;
	}

	if (data->SwapChain)
	{
		ID3D11Texture2D* pBackBuffer = nullptr;
		data->SwapChain->ResizeBuffers(0, static_cast<UINT>(size.x), static_cast<UINT>(size.y), DXGI_FORMAT_UNKNOWN, 0);
		data->SwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
		if (pBackBuffer == nullptr) { fprintf(stderr, __FUNCTION__"() failed creating buffers.\n"); return; }
		g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &data->RTView);
		pBackBuffer->Release();
	}
}

static void ImGui_ImplDX11_RenderWindow(ImGuiViewport* viewport, void*)
{
	ImGuiViewportDataDx11* data = static_cast<ImGuiViewportDataDx11*>(viewport->RendererUserData);
	ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	g_pd3dDeviceContext->OMSetRenderTargets(1, &data->RTView, nullptr);
	if (!(viewport->Flags & ImGuiViewportFlags_NoRendererClear))
		g_pd3dDeviceContext->ClearRenderTargetView(data->RTView, (float*)&clear_color);
	ImGui_ImplDX11_RenderDrawData(viewport->DrawData);
}

static void ImGui_ImplDX11_SwapBuffers(ImGuiViewport* viewport, void*)
{
	ImGuiViewportDataDx11* data = static_cast<ImGuiViewportDataDx11*>(viewport->RendererUserData);
	data->SwapChain->Present(0, 0); // Present without vsync
}

static void ImGui_ImplDX11_InitPlatformInterface()
{
	ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
	platform_io.Renderer_CreateWindow = ImGui_ImplDX11_CreateWindow;
	platform_io.Renderer_DestroyWindow = ImGui_ImplDX11_DestroyWindow;
	platform_io.Renderer_SetWindowSize = ImGui_ImplDX11_SetWindowSize;
	platform_io.Renderer_RenderWindow = ImGui_ImplDX11_RenderWindow;
	platform_io.Renderer_SwapBuffers = ImGui_ImplDX11_SwapBuffers;
}

static void ImGui_ImplDX11_ShutdownPlatformInterface()
{
	ImGui::DestroyPlatformWindows();
}
