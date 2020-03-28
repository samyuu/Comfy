#include "D3D_BlendState.h"
#include "Graphics/GraphicTypesNames.h"

namespace Comfy::Graphics
{
	namespace
	{
		constexpr UINT SampleMask = 0xFFFFFFFF;

		constexpr std::array<D3D11_BLEND, 4> GetBlendParameters(AetBlendMode blendMode)
		{
			switch (blendMode)
			{
			default:
				assert(false);

			case AetBlendMode::Normal:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::Add:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::Multiply:
				return { D3D11_BLEND_DEST_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::LinearDodge:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			case AetBlendMode::Overlay:
				return { D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ZERO, D3D11_BLEND_ONE };
			}
		}
	}

	D3D_BlendState::D3D_BlendState(AetBlendMode blendMode)
	{
		auto[sourceBlend, destinationBlend, sourceAlphaBlend, destinationAlphaBlend] = GetBlendParameters(blendMode);

		blendStateDescription.AlphaToCoverageEnable = false;
		blendStateDescription.IndependentBlendEnable = false;
		blendStateDescription.RenderTarget[0].BlendEnable = true;
		blendStateDescription.RenderTarget[0].SrcBlend = sourceBlend;
		blendStateDescription.RenderTarget[0].DestBlend = destinationBlend;
		blendStateDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].SrcBlendAlpha = sourceAlphaBlend;
		blendStateDescription.RenderTarget[0].DestBlendAlpha = destinationAlphaBlend;
		blendStateDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendStateDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		D3D.Device->CreateBlendState(&blendStateDescription, &blendState);

		D3D_SetObjectDebugName(GetBlendState(), "AetBlendMode: %s", AetBlendModeNames[static_cast<size_t>(blendMode)]);
	}

	D3D_BlendState::D3D_BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend)
		: D3D_BlendState(sourceBlend, destinationBlend, D3D11_BLEND_ZERO, D3D11_BLEND_ONE)
	{
	}

	D3D_BlendState::D3D_BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlpha, D3D11_BLEND destinationAlpha)
		: D3D_BlendState(sourceBlend, destinationBlend, sourceAlpha, destinationAlpha, D3D11_BLEND_OP_ADD, D3D11_BLEND_OP_ADD)
	{
	}

	D3D_BlendState::D3D_BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend,
		D3D11_BLEND_OP blendOp, D3D11_BLEND_OP blendAlphaOp, D3D11_COLOR_WRITE_ENABLE writeMask)
	{
		blendStateDescription.AlphaToCoverageEnable = false;
		blendStateDescription.IndependentBlendEnable = false;
		blendStateDescription.RenderTarget[0].BlendEnable = true;
		blendStateDescription.RenderTarget[0].SrcBlend = sourceBlend;
		blendStateDescription.RenderTarget[0].DestBlend = destinationBlend;
		blendStateDescription.RenderTarget[0].BlendOp = blendOp;
		blendStateDescription.RenderTarget[0].SrcBlendAlpha = sourceAlphaBlend;
		blendStateDescription.RenderTarget[0].DestBlendAlpha = destinationAlphaBlend;
		blendStateDescription.RenderTarget[0].BlendOpAlpha = blendAlphaOp;
		blendStateDescription.RenderTarget[0].RenderTargetWriteMask = writeMask;

		D3D.Device->CreateBlendState(&blendStateDescription, &blendState);
	}

	void D3D_BlendState::Bind()
	{
		D3D.Context->OMSetBlendState(blendState.Get(), nullptr, SampleMask);
	}

	void D3D_BlendState::UnBind()
	{
		D3D.Context->OMSetBlendState(nullptr, nullptr, SampleMask);
	}

	ID3D11BlendState* D3D_BlendState::GetBlendState()
	{
		return blendState.Get();
	}
}
