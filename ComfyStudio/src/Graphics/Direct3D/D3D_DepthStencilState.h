#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_DepthStencilState final : IGraphicsResource
	{
	public:
		D3D_DepthStencilState(bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask);
		D3D_DepthStencilState(bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask, const char* debugName);
		D3D_DepthStencilState(const D3D_DepthStencilState&) = default;
		~D3D_DepthStencilState() = default;

		D3D_DepthStencilState& operator=(const D3D_DepthStencilState&) = delete;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11DepthStencilState* GetDepthStencilState();

	private:
		D3D11_DEPTH_STENCIL_DESC depthStencilDescription;
		ComPtr<ID3D11DepthStencilState> depthStencilState;
	};
}
