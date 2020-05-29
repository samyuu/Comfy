#include "TextureSampler.h"

namespace Comfy::Render::D3D11
{
	namespace
	{
		constexpr u32 UnboundSamplerSlot = 0xFFFFFFFF;
	}

	TextureSampler::TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV)
		: TextureSampler(filter, addressModeUV, addressModeUV, 0.0f, D3D11_MIN_MAXANISOTROPY)
	{
	}
	
	TextureSampler::TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV)
		: TextureSampler(filter, addressModeU, addressModeV, 0.0f, D3D11_MIN_MAXANISOTROPY)
	{
	}

	TextureSampler::TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias, int anisotropicFiltering)
		: lastBoundSlot(UnboundSamplerSlot)
	{
		constexpr vec4 transparentBorderColor = vec4(0.0f, 0.0f, 0.0f, 0.0f);

		samplerDescription.Filter = filter;
		samplerDescription.AddressU = addressModeU;
		samplerDescription.AddressV = addressModeV;
		samplerDescription.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
		samplerDescription.MipLODBias = glm::clamp(mipMapBias, D3D11_MIP_LOD_BIAS_MIN, D3D11_MIP_LOD_BIAS_MAX);
		samplerDescription.MaxAnisotropy = anisotropicFiltering;
		samplerDescription.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
		samplerDescription.BorderColor[0] = transparentBorderColor[0];
		samplerDescription.BorderColor[1] = transparentBorderColor[1];
		samplerDescription.BorderColor[2] = transparentBorderColor[2];
		samplerDescription.BorderColor[3] = transparentBorderColor[3];
		samplerDescription.MinLOD = 0.0f;
		samplerDescription.MaxLOD = D3D11_FLOAT32_MAX;

		D3D.Device->CreateSamplerState(&samplerDescription, &samplerState);
	}

	void TextureSampler::Bind(u32 samplerSlot) const
	{
		assert(samplerSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);
		lastBoundSlot = samplerSlot;

		constexpr UINT samplerCount = 1;
		std::array<ID3D11SamplerState*, samplerCount> samplerStates = { samplerState.Get() };

		D3D.Context->PSSetSamplers(samplerSlot, samplerCount, samplerStates.data());
	}

	void TextureSampler::UnBind() const
	{
		assert(lastBoundSlot < D3D11_COMMONSHADER_SAMPLER_SLOT_COUNT);

		constexpr UINT samplerCount = 1;
		std::array<ID3D11SamplerState*, samplerCount> samplerStates = { nullptr };

		D3D.Context->PSSetSamplers(lastBoundSlot, samplerCount, samplerStates.data());
		lastBoundSlot = UnboundSamplerSlot;
	}
	
	ID3D11SamplerState* TextureSampler::GetSampler() const
	{
		return samplerState.Get();
	}
	
	const D3D11_SAMPLER_DESC& TextureSampler::GetDescription() const
	{
		return samplerDescription;
	}
}
