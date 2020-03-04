#pragma once
#include "../Direct3D.h"

namespace Comfy::Graphics
{
	class D3D_RasterizerState final : IGraphicsResource
	{
	public:
		D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled = false);
		D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled, const char* debugName);
		D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName);
		~D3D_RasterizerState() = default;

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
