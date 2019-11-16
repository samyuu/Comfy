#include "D3D_RasterizerState.h"

namespace Graphics
{
	D3D_RasterizerState::D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode)
	{
		rasterizerDescription.FillMode = fillMode;
		rasterizerDescription.CullMode = cullMode;
		rasterizerDescription.FrontCounterClockwise = false;
		rasterizerDescription.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		rasterizerDescription.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDescription.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDescription.DepthClipEnable = true;
		rasterizerDescription.ScissorEnable = false;
		rasterizerDescription.MultisampleEnable = false;
		rasterizerDescription.AntialiasedLineEnable = false;

		D3D.Device->CreateRasterizerState(&rasterizerDescription, &rasterizerState);
	}

	void D3D_RasterizerState::Bind()
	{
		D3D.Context->RSSetState(rasterizerState.Get());
	}

	void D3D_RasterizerState::UnBind()
	{
		D3D.Context->RSSetState(nullptr);
	}

	ID3D11RasterizerState* D3D_RasterizerState::GetRasterizerState()
	{
		return rasterizerState.Get();
	}
}
