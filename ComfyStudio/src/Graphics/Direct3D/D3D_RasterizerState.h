#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_RasterizerState final : IGraphicsResource
	{
	public:
		D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode);
		D3D_RasterizerState(const D3D_RasterizerState&) = default;
		~D3D_RasterizerState() = default;

		D3D_RasterizerState& operator=(const D3D_RasterizerState&) = delete;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11RasterizerState* GetRasterizerState();

	private:
		D3D11_RASTERIZER_DESC rasterizerDescription;
		ComPtr<ID3D11RasterizerState> rasterizerState;
	};
}
