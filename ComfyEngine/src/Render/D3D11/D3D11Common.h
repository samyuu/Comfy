#pragma once
#include "Types.h"
#include "Core/Win32LeanWindowsHeader.h"

#include <d3d11_1.h>
#include <wrl.h>

using Microsoft::WRL::ComPtr;

// NOTE: A set of helper functions for working with raw ID3D11{Interface} pointers and reduce boilerplate code
namespace Comfy::Render::D3D11Helper
{
	// TODO: Make proper Rect and RectF structs..?
	using Rect = ivec4;

	constexpr D3D11_RECT RectToD3DRect(Rect rect) { return { rect.x, rect.y, (rect.x + rect.z), (rect.y + rect.w) }; }
	constexpr Rect D3DRectToRect(D3D11_RECT rect) { return { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top }; }

	bool HandleSwapChainResizeIfNeeded(ID3D11Device* device, IDXGISwapChain* inSwapChain, ComPtr<ID3D11Texture2D>& inOutSwapChainFB, ComPtr<ID3D11RenderTargetView>& inOutSwapChainFBView,
		ivec2 newWindowSize);

	void CreateSimpleDepthBuffer(ID3D11Device* device, ComPtr<ID3D11Texture2D>& outDepthTexture, ComPtr<ID3D11DepthStencilView>& outDepthStencilView,
		u32 width, u32 height, DXGI_FORMAT format);

	void CreateVertexBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& outVertexBuffer,
		size_t byteSize, const void* initialData, D3D11_USAGE usage);

	void CreateIndexBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& outIndexBuffer,
		size_t byteSize, const void* initialData, D3D11_USAGE usage);

	void CreateSimpleDepthStencilState(ID3D11Device* device, ComPtr<ID3D11DepthStencilState>& outDepthStencilState,
		bool depthEnable, D3D11_DEPTH_WRITE_MASK depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL, D3D11_COMPARISON_FUNC depthFunc = D3D11_COMPARISON_LESS);

	void CreateConstantBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& outConstantBuffer,
		size_t byteSize);

	void CreateSimpleRasterizerState(ID3D11Device* device, ComPtr<ID3D11RasterizerState>& outRasterizerState,
		D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnable);

	void CreateSimpleBlendState(ID3D11Device* device, ComPtr<ID3D11BlendState>& outBlendState,
		D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend);

	void CreateSimpleBlendState(ID3D11Device* device, ComPtr<ID3D11BlendState>& outBlendState,
		D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend);

	void CreateSimpleSamplerState(ID3D11Device* device, ComPtr<ID3D11SamplerState>& outSamplerState,
		D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV, vec4 borderColor = { 1.0f, 1.0f, 1.0f, 1.0f });

	void CreateSimpleTexture2DAndView(ID3D11Device* device, ComPtr<ID3D11Texture2D>& outTexture, ComPtr<ID3D11ShaderResourceView>& outTextureView,
		ivec2 size, size_t stride, const void* initialData, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM, D3D11_USAGE usage = D3D11_USAGE_IMMUTABLE);

	void CreateSimpleTexture2DAndViewAndGenerateMipMaps(ID3D11Device* device, ID3D11DeviceContext* context, ComPtr<ID3D11Texture2D>& outTexture, ComPtr<ID3D11ShaderResourceView>& outTextureView,
		ivec2 size, size_t stride, const void* initialData, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	template <typename Func>
	inline void MemoryMap(ID3D11DeviceContext* context, ID3D11Resource* resource, Func func, D3D11_MAP map = D3D11_MAP_WRITE_DISCARD)
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubresource = {};
		HRESULT result = context->Map(resource, 0, map, 0, &mappedSubresource);
		assert(SUCCEEDED(result) && mappedSubresource.pData != nullptr);

		if (mappedSubresource.pData != nullptr)
			func(mappedSubresource.pData);

		context->Unmap(resource, 0);
	}

	inline void MemoryMapAndCopy(ID3D11DeviceContext* context, ID3D11Resource* resource, const void* dataToCopy, size_t dataSize)
	{
		MemoryMap(context, resource, [&](void* data)
		{
			memcpy(data, dataToCopy, dataSize);
		}, D3D11_MAP_WRITE_DISCARD);
	}
}
