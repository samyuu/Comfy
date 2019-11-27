#include "D3D_TextureSampler.h"

namespace Graphics
{
	namespace
	{
		constexpr uint32_t UnboundSamplerSlot = 0xFFFFFFFF;
	}

	D3D_TextureSampler::D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV)
		: D3D_TextureSampler(filter, addressModeUV, addressModeUV, 0.0f)
	{
	}
	
	D3D_TextureSampler::D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV)
		: D3D_TextureSampler(filter, addressModeU, addressModeV, 0.0f)
	{
	}

	D3D_TextureSampler::D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias)
		: lastBoundSlot(UnboundSamplerSlot)
	{
		constexpr vec4 transparentBorderColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);

		samplerDescription.Filter = filter;
		samplerDescription.AddressU = addressModeU;
		samplerDescription.AddressV = addressModeV;
		samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDescription.MipLODBias = glm::clamp(mipMapBias, D3D11_MIP_LOD_BIAS_MIN, D3D11_MIP_LOD_BIAS_MAX);
		samplerDescription.MaxAnisotropy = 0;
		samplerDescription.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDescription.BorderColor[0] = transparentBorderColor[0];
		samplerDescription.BorderColor[1] = transparentBorderColor[1];
		samplerDescription.BorderColor[2] = transparentBorderColor[2];
		samplerDescription.BorderColor[3] = transparentBorderColor[3];
		samplerDescription.MinLOD = 0.0f;
		samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

		D3D.Device->CreateSamplerState(&samplerDescription, &samplerState);
	}

	void D3D_TextureSampler::Bind(uint32_t samplerSlot) const
	{
		assert(samplerSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
		lastBoundSlot = samplerSlot;

		constexpr UINT samplerCount = 1;
		std::array<ID3D11SamplerState*, samplerCount> samplerStates = { samplerState.Get() };

		D3D.Context->PSSetSamplers(samplerSlot, samplerCount, samplerStates.data());
	}

	void D3D_TextureSampler::UnBind() const
	{
		assert(lastBoundSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

		constexpr UINT samplerCount = 1;
		std::array<ID3D11SamplerState*, samplerCount> samplerStates = { nullptr };

		D3D.Context->PSSetSamplers(lastBoundSlot, samplerCount, samplerStates.data());
		lastBoundSlot = UnboundSamplerSlot;
	}
	
	ID3D11SamplerState* D3D_TextureSampler::GetSampler()
	{
		return samplerState.Get();
	}
	
	const D3D11_SAMPLER_DESC& D3D_TextureSampler::GetDescription() const
	{
		return samplerDescription;
	}
}
