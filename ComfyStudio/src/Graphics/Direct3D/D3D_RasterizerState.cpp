#include "D3D_RasterizerState.h"

namespace Graphics
{
	D3D_RasterizerState::D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode)
	{
		rasterizerDescription.FillMode = fillMode;
		rasterizerDescription.CullMode = cullMode;
		// NOTE: Default to true because ObjSet files were originally design to be used with OpenGL
		rasterizerDescription.FrontCounterClockwise = true;
		rasterizerDescription.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		rasterizerDescription.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDescription.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDescription.DepthClipEnable = true;
		rasterizerDescription.ScissorEnable = false;
		rasterizerDescription.MultisampleEnable = false;
		rasterizerDescription.AntialiasedLineEnable = false;

		D3D.Device->CreateRasterizerState(&rasterizerDescription, &rasterizerState);
	}

	D3D_RasterizerState::D3D_RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName)
		: D3D_RasterizerState(fillMode, cullMode)
	{
		D3D_SetObjectDebugName(rasterizerState.Get(), debugName);
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
