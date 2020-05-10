#pragma once
#include "../Direct3D.h"

namespace Comfy::Render::D3D11
{
	class RasterizerState final : IGraphicsResource
	{
	public:
		RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled = false);
		RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled, const char* debugName);
		RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName);
		~RasterizerState() = default;

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
