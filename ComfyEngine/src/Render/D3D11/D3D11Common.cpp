#include "D3D11Common.h"

namespace Comfy::Render::D3D11Helper
{
	bool HandleSwapChainResizeIfNeeded(ID3D11Device* device, IDXGISwapChain* inSwapChain, ComPtr<ID3D11Texture2D>& inOutSwapChainFB, ComPtr<ID3D11RenderTargetView>& inOutSwapChainFBView,
		ivec2 newWindowSize)
	{
		DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
		inSwapChain->GetDesc(&swapChainDesc);

		D3D11_TEXTURE2D_DESC swapChainFBDesc = {};
		inOutSwapChainFB->GetDesc(&swapChainFBDesc);

		if (swapChainFBDesc.Width == newWindowSize.x && swapChainFBDesc.Height == newWindowSize.y)
			return false;

		inOutSwapChainFBView = nullptr;
		inOutSwapChainFB = nullptr;

		HRESULT hr;
		hr = inSwapChain->ResizeBuffers(swapChainDesc.BufferCount, newWindowSize.x, newWindowSize.y, DXGI_FORMAT_UNKNOWN, swapChainDesc.Flags);
		hr = inSwapChain->GetBuffer(0, __uuidof(inOutSwapChainFB), &inOutSwapChainFB);
		hr = device->CreateRenderTargetView(inOutSwapChainFB.Get(), nullptr, &inOutSwapChainFBView);
		return true;
	}

	void CreateSimpleDepthBuffer(ID3D11Device* device, ComPtr<ID3D11Texture2D>& outDepthTexture, ComPtr<ID3D11DepthStencilView>& outDepthStencilView,
		u32 width, u32 height, DXGI_FORMAT format)
	{
		D3D11_TEXTURE2D_DESC depthBufferDesc = {};
		depthBufferDesc.Width = width;
		depthBufferDesc.Height = height;
		depthBufferDesc.MipLevels = 1;
		depthBufferDesc.ArraySize = 1;
		depthBufferDesc.Format = format;
		depthBufferDesc.SampleDesc.Count = 1;
		depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

		HRESULT hr;
		hr = device->CreateTexture2D(&depthBufferDesc, nullptr, &outDepthTexture);
		assert(SUCCEEDED(hr));
		hr = device->CreateDepthStencilView(outDepthTexture.Get(), nullptr, &outDepthStencilView);
		assert(SUCCEEDED(hr));
	}

	void CreateVertexBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& outVertexBuffer,
		size_t byteSize, const void* initialData, D3D11_USAGE usage)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = static_cast<UINT>(byteSize);
		bufferDesc.Usage = usage;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		if (usage != D3D11_USAGE_IMMUTABLE)
			bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

		const D3D11_SUBRESOURCE_DATA initialSubresource = { initialData };
		HRESULT hr = device->CreateBuffer(&bufferDesc, (initialData != nullptr) ? &initialSubresource : nullptr, &outVertexBuffer);
		assert(SUCCEEDED(hr));
	}

	void CreateIndexBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& outIndexBuffer,
		size_t byteSize, const void* initialData, D3D11_USAGE usage)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = static_cast<UINT>(byteSize);
		bufferDesc.Usage = usage;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		if (usage != D3D11_USAGE_IMMUTABLE)
			bufferDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

		const D3D11_SUBRESOURCE_DATA initialSubresource = { initialData };
		HRESULT hr = device->CreateBuffer(&bufferDesc, (initialData != nullptr) ? &initialSubresource : nullptr, &outIndexBuffer);
		assert(SUCCEEDED(hr));
	}

	void CreateSimpleDepthStencilState(ID3D11Device* device, ComPtr<ID3D11DepthStencilState>& outDepthStencilState,
		bool depthEnable, D3D11_DEPTH_WRITE_MASK depthWriteMask, D3D11_COMPARISON_FUNC depthFunc)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = depthEnable;
		depthStencilDesc.DepthWriteMask = depthWriteMask;
		depthStencilDesc.DepthFunc = depthFunc;

		HRESULT hr = device->CreateDepthStencilState(&depthStencilDesc, &outDepthStencilState);
		assert(SUCCEEDED(hr));
	}

	void CreateConstantBuffer(ID3D11Device* device, ComPtr<ID3D11Buffer>& outConstantBuffer,
		size_t byteSize)
	{
		D3D11_BUFFER_DESC bufferDesc = {};
		bufferDesc.ByteWidth = static_cast<UINT>(byteSize) + 0xF & 0xFFFFFFF0; // NOTE: Round to 16 byte boundary
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = device->CreateBuffer(&bufferDesc, nullptr, &outConstantBuffer);
		assert(SUCCEEDED(hr));
	}

	void CreateSimpleRasterizerState(ID3D11Device* device, ComPtr<ID3D11RasterizerState>& outRasterizerState,
		D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnable)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};
		rasterizerDesc.FillMode = fillMode;
		rasterizerDesc.CullMode = cullMode;
		rasterizerDesc.ScissorEnable = scissorEnable;

		HRESULT hr = device->CreateRasterizerState(&rasterizerDesc, &outRasterizerState);
		assert(SUCCEEDED(hr));
	}

	void CreateSimpleBlendState(ID3D11Device* device, ComPtr<ID3D11BlendState>& outBlendState,
		D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend)
	{
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.AlphaToCoverageEnable = false;
		blendDesc.IndependentBlendEnable = false;
		blendDesc.RenderTarget[0].BlendEnable = true;
		blendDesc.RenderTarget[0].SrcBlend = sourceBlend;
		blendDesc.RenderTarget[0].DestBlend = destinationBlend;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = sourceAlphaBlend;
		blendDesc.RenderTarget[0].DestBlendAlpha = destinationAlphaBlend;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		HRESULT hr = device->CreateBlendState(&blendDesc, &outBlendState);
		assert(SUCCEEDED(hr));
	}

	void CreateSimpleBlendState(ID3D11Device* device, ComPtr<ID3D11BlendState>& outBlendState,
		D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend)
	{
		return CreateSimpleBlendState(device, outBlendState, sourceBlend, destinationBlend, D3D11_BLEND_ONE, D3D11_BLEND_ZERO);
	}

	void CreateSimpleSamplerState(ID3D11Device* device, ComPtr<ID3D11SamplerState>& outSamplerState,
		D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressU, D3D11_TEXTURE_ADDRESS_MODE addressV, vec4 borderColor)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};
		samplerDesc.Filter = filter;
		samplerDesc.AddressU = addressU;
		samplerDesc.AddressV = addressV;
		samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDesc.MipLODBias = 0.0f;
		samplerDesc.MaxAnisotropy = 1;
		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.BorderColor[0] = borderColor[0];
		samplerDesc.BorderColor[1] = borderColor[1];
		samplerDesc.BorderColor[2] = borderColor[2];
		samplerDesc.BorderColor[3] = borderColor[3];
		samplerDesc.MinLOD = -FLT_MAX;
		samplerDesc.MaxLOD = +FLT_MAX;

		HRESULT hr = device->CreateSamplerState(&samplerDesc, &outSamplerState);
		assert(SUCCEEDED(hr));
	}

	void CreateSimpleTexture2DAndView(ID3D11Device* device, ComPtr<ID3D11Texture2D>& outTexture, ComPtr<ID3D11ShaderResourceView>& outTextureView,
		ivec2 size, size_t stride, const void* initialData, DXGI_FORMAT format, D3D11_USAGE usage)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = size.x;
		textureDesc.Height = size.y;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = usage;
		textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		if (usage != D3D11_USAGE_IMMUTABLE)
			textureDesc.CPUAccessFlags |= D3D11_CPU_ACCESS_WRITE;

		D3D11_SUBRESOURCE_DATA initialSubresource = {};
		initialSubresource.pSysMem = initialData;
		initialSubresource.SysMemPitch = static_cast<UINT>(stride);

		HRESULT hr;
		hr = device->CreateTexture2D(&textureDesc, &initialSubresource, &outTexture);
		assert(SUCCEEDED(hr));
		hr = device->CreateShaderResourceView(outTexture.Get(), nullptr, &outTextureView);
		assert(SUCCEEDED(hr));
	}

	void CreateSimpleTexture2DAndViewAndGenerateMipMaps(ID3D11Device* device, ID3D11DeviceContext* context, ComPtr<ID3D11Texture2D>& outTexture, ComPtr<ID3D11ShaderResourceView>& outTextureView,
		ivec2 size, size_t stride, const void* initialData, DXGI_FORMAT format)
	{
		D3D11_TEXTURE2D_DESC textureDesc = {};
		textureDesc.Width = size.x;
		textureDesc.Height = size.y;
		textureDesc.MipLevels = 0;
		textureDesc.ArraySize = 1;
		textureDesc.Format = format;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = (D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET);
		textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		HRESULT hr;
		hr = device->CreateTexture2D(&textureDesc, nullptr, &outTexture);
		assert(SUCCEEDED(hr));
		hr = device->CreateShaderResourceView(outTexture.Get(), nullptr, &outTextureView);
		assert(SUCCEEDED(hr));

		context->UpdateSubresource(outTexture.Get(), 0, nullptr, initialData, static_cast<UINT>(stride), static_cast<UINT>(stride * size.y));
		context->GenerateMips(outTextureView.Get());
	}
}
