#pragma once
#include "Types.h"
#include "../Direct3D.h"

namespace Comfy::Graphics::D3D11
{
	class TextureSampler : IGraphicsResource
	{
	public:
		TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV);
		TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV);
		TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias, int anisotropicFiltering);
		virtual ~TextureSampler() = default;
		
	public:
		virtual void Bind(u32 samplerSlot) const;
		virtual void UnBind() const;

	public:
		template <size_t Size>
		static void BindArray(u32 startSlot, const std::array<TextureSampler*, Size>& samplers);

	public:
		ID3D11SamplerState* GetSampler();
		const D3D11_SAMPLER_DESC& GetDescription() const;

	protected:
		mutable u32 lastBoundSlot;
		D3D11_SAMPLER_DESC samplerDescription;

		ComPtr<ID3D11SamplerState> samplerState;
	};

	template<size_t Size>
	void TextureSampler::BindArray(u32 startSlot, const std::array<TextureSampler*, Size>& samplers)
	{
		std::array<ID3D11SamplerState*, Size> samplerStates;

		for (size_t i = 0; i < Size; i++)
			samplerStates[i] = (samplers[i] != nullptr) ? samplers[i]->GetSampler() : nullptr;

		D3D.Context->PSSetSamplers(startSlot, static_cast<UINT>(samplerStates.size()), samplerStates.data());
	}
}
