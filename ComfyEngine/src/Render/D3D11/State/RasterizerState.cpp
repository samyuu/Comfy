#include "RasterizerState.h"

namespace Comfy::Render::D3D11
{
	RasterizerState::RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled)
	{
		rasterizerDescription.FillMode = fillMode;
		rasterizerDescription.CullMode = cullMode;
		// NOTE: Default to true because ObjSet files were originally design to be used with OpenGL
		rasterizerDescription.FrontCounterClockwise = true;
		rasterizerDescription.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
		rasterizerDescription.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
		rasterizerDescription.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		rasterizerDescription.DepthClipEnable = true;
		rasterizerDescription.ScissorEnable = scissorEnabled;
		rasterizerDescription.MultisampleEnable = false;
		rasterizerDescription.AntialiasedLineEnable = false;

		D3D.Device->CreateRasterizerState(&rasterizerDescription, &rasterizerState);
	}

	RasterizerState::RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, const char* debugName)
		: RasterizerState(fillMode, cullMode, false, debugName)
	{
	}

	RasterizerState::RasterizerState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode, bool scissorEnabled, const char* debugName)
		: RasterizerState(fillMode, cullMode, scissorEnabled)
	{
		D3D11_SetObjectDebugName(rasterizerState.Get(), debugName);
	}

	void RasterizerState::Bind()
	{
		D3D.Context->RSSetState(rasterizerState.Get());
	}

	void RasterizerState::UnBind()
	{
		D3D.Context->RSSetState(nullptr);
	}

	ID3D11RasterizerState* RasterizerState::GetRasterizerState()
	{
		return rasterizerState.Get();
	}
}
