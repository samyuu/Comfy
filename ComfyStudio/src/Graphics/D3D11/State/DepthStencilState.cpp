#include "DepthStencilState.h"

namespace Comfy::Graphics
{
	D3D_DepthStencilState::D3D_DepthStencilState(bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask)
	{
		depthStencilDescription.DepthEnable = depthEnabled;
		depthStencilDescription.DepthWriteMask = depthWriteMask;
		depthStencilDescription.DepthFunc = depthEnabled ? D3D11_COMPARISON_LESS_EQUAL : D3D11_COMPARISON_ALWAYS;
		depthStencilDescription.StencilEnable = false;
		depthStencilDescription.StencilReadMask = 0xFF;
		depthStencilDescription.StencilWriteMask = 0xFF;
		depthStencilDescription.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDescription.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDescription.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDescription.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthStencilDescription.BackFace = depthStencilDescription.FrontFace;

		D3D.Device->CreateDepthStencilState(&depthStencilDescription, &depthStencilState);
	}

	D3D_DepthStencilState::D3D_DepthStencilState(bool depthEnabled, D3D11_DEPTH_WRITE_MASK depthWriteMask, const char* debugName)
		: D3D_DepthStencilState(depthEnabled, depthWriteMask)
	{
		D3D_SetObjectDebugName(depthStencilState.Get(), debugName);
	}

	void D3D_DepthStencilState::Bind()
	{
		D3D.Context->OMSetDepthStencilState(depthStencilState.Get(), 0);
	}

	void D3D_DepthStencilState::UnBind()
	{
		D3D.Context->OMSetDepthStencilState(nullptr, 0);
	}
	
	ID3D11DepthStencilState* D3D_DepthStencilState::GetDepthStencilState()
	{
		return depthStencilState.Get();
	}
}
