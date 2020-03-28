#pragma once
#include "../Direct3D.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Graphics
{
	class D3D_BlendState final : ID3DGraphicsResource
	{
	public:
		D3D_BlendState(AetBlendMode blendMode);

		D3D_BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend);
		D3D_BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlpha, D3D11_BLEND destinationAlpha);

		D3D_BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend,
			D3D11_BLEND_OP blendOp, D3D11_BLEND_OP blendAlphaOp, D3D11_COLOR_WRITE_ENABLE writeMask = D3D11_COLOR_WRITE_ENABLE_ALL);

		~D3D_BlendState() = default;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11BlendState* GetBlendState();

	private:
		D3D11_BLEND_DESC blendStateDescription;
		ComPtr<ID3D11BlendState> blendState;
	};
}
