#pragma once
#include "../Direct3D.h"

namespace Comfy::Graphics::D3D11
{
	class DepthStencilState final : IGraphicsResource
	{
	public:
		DepthStencilState(bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask);
		DepthStencilState(bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask, const char* debugName);
		~DepthStencilState() = default;

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
