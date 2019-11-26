#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_DepthBuffer final : IGraphicsResource
	{
	public:
		D3D_DepthBuffer(ivec2 size, DXGI_FORMAT format);
		D3D_DepthBuffer(const D3D_DepthBuffer&) = delete;
		~D3D_DepthBuffer() = default;

		D3D_DepthBuffer& operator=(const D3D_DepthBuffer&) = delete;

	public:
		void Clear(float value = 1.0f);
		void Resize(ivec2 newSize);

	public:
		ivec2 GetSize() const;
		ID3D11DepthStencilView* GetDepthStencilView() const;

	private:
		D3D11_TEXTURE2D_DESC textureDescription;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDescription;

		ComPtr<ID3D11Texture2D> depthTexture;
		ComPtr<ID3D11DepthStencilView> depthStencilView;
	};
}
