#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_TextureSampler : IGraphicsResource
	{
	public:
		D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV);
		D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV);
		D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias, int anisotropicFiltering);
		D3D_TextureSampler(const D3D_TextureSampler&) = delete;
		virtual ~D3D_TextureSampler() = default;
		
		D3D_TextureSampler& operator=(const D3D_TextureSampler&) = delete;

	public:
		virtual void Bind(uint32_t samplerSlot) const;
		virtual void UnBind() const;

	public:
		ID3D11SamplerState* GetSampler();
		const D3D11_SAMPLER_DESC& GetDescription() const;

	protected:
		mutable uint32_t lastBoundSlot;
		D3D11_SAMPLER_DESC samplerDescription;

		ComPtr<ID3D11SamplerState> samplerState;
	};
}
