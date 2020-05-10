#pragma once
#include "../Direct3D.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Graphics::D3D11
{
	class BlendState final : IGraphicsResource
	{
	public:
		BlendState(AetBlendMode blendMode);

		BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend);
		BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlpha, D3D11_BLEND destinationAlpha);

		BlendState(D3D11_BLEND sourceBlend, D3D11_BLEND destinationBlend, D3D11_BLEND sourceAlphaBlend, D3D11_BLEND destinationAlphaBlend,
			D3D11_BLEND_OP blendOp, D3D11_BLEND_OP blendAlphaOp, D3D11_COLOR_WRITE_ENABLE writeMask = D3D11_COLOR_WRITE_ENABLE_ALL);

		~BlendState() = default;

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
